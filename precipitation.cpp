#include "precipitation.h"

void Precipitation::calculate_particle_cutoff_point(Precipitation::Particle *p_particle, const AABB &p_box, const Vector3 &p_wind_velocity) {
	if (using_collision == true) {
		Vector3 velocity = p_wind_velocity / p_particle->mass - Vector3(0, p_particle->velocity, 0);
		velocity = velocity.normalized();
		
		Vector3 from = p_particle->position;
		Vector3 to = p_particle->position + (velocity * 100);

		Set<RID> exclude;

		PhysicsDirectSpaceState::RayResult rr;
		if (dss->intersect_ray(from, to, rr, exclude, collision_mask, PhysicsDirectSpaceState::TYPE_MASK_STATIC_BODY))
			p_particle->hit_pos = rr.position;
		else
			p_particle->hit_pos = Vector3(0,-1000,0);

		p_particle->is_valid = p_particle->position.y > p_particle->hit_pos.y;
	}
	else {
		p_particle->hit_pos = Vector3(0, -1000, 0);
		p_particle->is_valid = true;
	}
}

void Precipitation::spawn_particle(Precipitation::Particle *p_particle, const float p_delta) {
	p_particle->velocity = Math::randf() * (max_speed - min_speed) + min_speed;

	p_particle->position.x = visibility_box.pos.x + Math::randf() * box_size.x - (box_size.x / 2);
	p_particle->position.z = visibility_box.pos.z + Math::randf() * box_size.x - (box_size.x / 2);

	p_particle->anim_start_time = (p_delta * Math::randf());

	p_particle->tex_coord_index = (int)(Math::randf() * (float)(drops_per_texture * drops_per_texture - 0.5));

	p_particle->is_valid = true;
	p_particle->time = Math::randf() * Math_PI * 2.0;
	p_particle->mass = Math::randf() * (max_mass - min_mass) + min_mass;
}

void Precipitation::spawn_new_particle(Precipitation::Particle *p_particle, const float p_delta) {
	spawn_particle(p_particle, p_delta);
	p_particle->position.y = Math::randf() * box_size.y - (box_size.y / 2);
}

void Precipitation::empty_particles() {
	Particle *current = particle_head;
	while (current) {
		Particle *next = current->next;
		memfree(current);
		current = next;
	}
	particle_head = NULL;
}

void Precipitation::populate_particles(const float p_delta) {
	int new_particle_count = (max_particles * percentage);
	int particle_count = 0;
	
	if(new_particle_count == 0)
		empty_particles();

	if (particle_head) {
		Particle *current = particle_head;
		while (current) {
			particle_count += 1;
			current = current->next;
			if (particle_count == new_particle_count && current) {
				Particle *next = current->next;
				current->next = NULL;
				while (next) {
					Particle *last = next;
					next = next->next;
					last->next = NULL;
					memfree(last);
				}
				break;
			}
		}
	}

	if (particle_count < new_particle_count) {
		Particle *current = particle_head;
		if (current) {
			while (current->next)
				current = current->next;
		}
		else{
			current = memnew(Particle);
			particle_head = current;
			spawn_new_particle(current, p_delta);
			particle_count += 1;
		}
		while (particle_count < new_particle_count) {
			current->next = memnew(Particle);
			current = current->next;
			spawn_new_particle(current, p_delta);
			particle_count += 1;
		}
	}
}

void Precipitation::wrap_particle(Precipitation::Particle *p_particle, AABB &p_box, Vector3 &p_wind_velocity, const float p_delta) {
	if (p_particle->position.y < (p_box.pos.y - (p_box.size.y * 0.5))) {
		spawn_particle(p_particle, p_delta);
		while (p_particle->position.y < (p_box.pos.y - (p_box.size.y * 0.5)))
			p_particle->position.y += box_size.y;
		calculate_particle_cutoff_point(p_particle, p_box, p_wind_velocity);
	}
	else if (p_particle->position.y >(p_box.pos.y + (p_box.size.y * 0.5))) {
		while (p_particle->position.y >(p_box.pos.y + (p_box.size.y * 0.5))) {
			p_particle->position.y -= box_size.y;
		}
		calculate_particle_cutoff_point(p_particle, p_box, p_wind_velocity);
	}
	else if (p_particle->position.x < (p_box.pos.x - (p_box.size.x * 0.5))) {
		while (p_particle->position.x < (p_box.pos.x - (p_box.size.x * 0.5))) {
			p_particle->position.x += box_size.x;
		}
		calculate_particle_cutoff_point(p_particle, p_box, p_wind_velocity);
	}
	else if (p_particle->position.x >(p_box.pos.x + (p_box.size.x * 0.5))) {
		while (p_particle->position.x >(p_box.pos.x + (p_box.size.x * 0.5))) {
			p_particle->position.x -= box_size.x;
		}
		calculate_particle_cutoff_point(p_particle, p_box, p_wind_velocity);
	}
	else if (p_particle->position.z < (p_box.pos.z - (p_box.size.z * 0.5))) {
		while (p_particle->position.z < (p_box.pos.z - (p_box.size.z * 0.5))) {
			p_particle->position.z += box_size.x;
		}
		calculate_particle_cutoff_point(p_particle, p_box, p_wind_velocity);
	}
	else if (p_particle->position.z >(p_box.pos.z + (p_box.size.z * 0.5))) {
		while (p_particle->position.z >(p_box.pos.z + (p_box.size.z * 0.5))) {
			p_particle->position.z -= box_size.x;
		}
		calculate_particle_cutoff_point(p_particle, p_box, p_wind_velocity);
	}
}

void Precipitation::_notification(int p_what) {
	switch(p_what) {

		case NOTIFICATION_ENTER_TREE: {
			immediate_geometry = memnew( ImmediateGeometry );
			add_child(immediate_geometry);

			immediate_geometry->set_layer_mask(visibility_mask);
			if (drop_particle_material.is_valid()) {
				immediate_geometry->set_material_override(drop_particle_material);
			}

			visibility_box = AABB(Vector3(), Vector3(box_size.x, box_size.y, box_size.x));
			pending_update = true;
		} break;
		case NOTIFICATION_READY: {
			if (has_node(camera_path)) {
				camera_node = static_cast<Camera *>(get_node(camera_path));
			}
			else {
				camera_node = NULL;
			}

			set_fixed_process(true);

			w3d = get_world();
			ERR_BREAK(w3d.is_null());

			dss = PhysicsServer::get_singleton()->space_get_direct_state(w3d->get_space());
			ERR_BREAK(!dss);
		} break;
		case NOTIFICATION_EXIT_TREE: {
		} break;
		case NOTIFICATION_FIXED_PROCESS: {
			if (drop_particle_material.is_valid() && camera_node) {
				ShaderMaterial *shader_material = static_cast<ShaderMaterial *>(*drop_particle_material);
				if (shader_material)
					shader_material->set_shader_param("CameraPosition", camera_node->get_global_transform().origin);
			}

			if (camera_node)
				visibility_box.set_pos(camera_node->get_global_transform().origin);
			else
				visibility_box.set_pos(Vector3());


			_precipitation_process(get_fixed_process_delta_time());

			draw_particles();

			break;
		}
	}
}

void Precipitation::update_render_cache() {
	cached_coordinates.resize(drops_per_texture * drops_per_texture * 4);
	float uv_size = 1.0 / drops_per_texture;

	for (int i = 0; i < (drops_per_texture * drops_per_texture); i++) {
		int index = i * 4;

		int x_modulo = (i % drops_per_texture);
		float x_uv_position = uv_size * x_modulo;
		float y_uv_position = uv_size * (i - x_modulo) / drops_per_texture;

		cached_coordinates[index] = Vector2(x_uv_position, y_uv_position);
		cached_coordinates[index + 1] = Vector2(x_uv_position + uv_size, y_uv_position);
		cached_coordinates[index + 2] = Vector2(x_uv_position, y_uv_position + uv_size);
		cached_coordinates[index + 3] = Vector2(x_uv_position + uv_size, y_uv_position + uv_size);
	}
}

void Precipitation::_precipitation_process(const float p_delta) {
	if (is_hidden())
		return;

	if (pending_update) {
		populate_particles(p_delta);
		update_render_cache();
		pending_update = false;
	}

	if (camera_node == NULL)
		return;

	Vector3 cam_pos = camera_node->get_global_transform().origin;
	Matrix3 cam_mat = camera_node->get_global_transform().basis;
	float cam_fov = camera_node->get_fov();
	Vector3 cam_dir = cam_mat.elements[1].normalized();

	Particle *curr = particle_head;
	while (curr) {
		curr->position += wind_velocity / curr->mass;
		curr->position.y -= curr->velocity;

		wrap_particle(curr, visibility_box, wind_velocity, p_delta);

		if (curr->is_valid && curr->position.y < curr->hit_pos.y) {
			curr->is_valid = false;
		}

		curr->to_render = true;
		curr->render_position = curr->position;

		curr = curr->next;
	}
}

void Precipitation::draw_particles() {
	Vector3 pos;
	Matrix3 cam_mat;
	Vector3 cam_origin;
	Vector3 right;
	Vector3 up;
	Vector3 right_up;
	Vector3 left_up;
	float distance;
	Vector3 ortho_dir;
	Vector3 velocity;
	
	Particle *curr = particle_head;
	uint32_t vert_count = 0;
	
	cam_origin = camera_node->get_global_transform().origin;
	
	if (using_billboards) {
		cam_mat = camera_node->get_global_transform().basis;
		right = cam_mat[0].normalized() * drop_particle_size;
		up = cam_mat[1].normalized() * drop_particle_size;
		right_up = right + up;
		left_up = -right + up;
	}
	
	immediate_geometry->clear();
	immediate_geometry->begin(Mesh::PRIMITIVE_TRIANGLES, NULL);
	
	while (curr) {
		if (!curr->is_valid || !curr->to_render) {
			curr = curr->next;
			continue;
		}
		pos = curr->render_position;
		
		if (using_billboards == false) {
			cam_origin.y = pos.y;
			ortho_dir = cam_origin - pos;
			distance = ortho_dir.length();
			
			if (distance > 0.0)
				ortho_dir *= -1.0 / distance;
			else
				ortho_dir = -Vector3(0, 0, 1);
			
			velocity = wind_velocity / curr->mass;
			velocity.z -= curr->velocity;
			velocity = velocity.normalized();
			
			up = (-velocity.cross(ortho_dir)).normalized() * drop_particle_size;
			right = (ortho_dir.cross(up) - velocity).normalized() * drop_particle_size;
			right_up = right + up;
			left_up = -right + up;
		}
			
		uint32_t index = curr->tex_coord_index * 4;
			
		immediate_geometry->set_uv(cached_coordinates[index]);
		immediate_geometry->add_vertex(pos + left_up);
		vert_count += 1;
		
		immediate_geometry->set_uv(cached_coordinates[index + 1]);
		immediate_geometry->add_vertex(pos + right_up);
		vert_count += 1;
		
		immediate_geometry->set_uv(cached_coordinates[index + 3]);
		immediate_geometry->add_vertex(pos - left_up);
		vert_count += 1;
		
		immediate_geometry->set_uv(cached_coordinates[index + 3]);
		immediate_geometry->add_vertex(pos - left_up);
		vert_count += 1;
		
		immediate_geometry->set_uv(cached_coordinates[index + 2]);
		immediate_geometry->add_vertex(pos - right_up);
		vert_count += 1;

		immediate_geometry->set_uv(cached_coordinates[index]);
		immediate_geometry->add_vertex(pos + left_up);
		vert_count += 1;
		
		curr = curr->next;
	}
			
	immediate_geometry->end();
}

void Precipitation::_bind_methods() {
	ObjectTypeDB::bind_method(_MD("set_camera", "camera_path"), &Precipitation::set_camera);
	ObjectTypeDB::bind_method(_MD("get_camera"), &Precipitation::get_camera);

	ObjectTypeDB::bind_method(_MD("set_collision_mask", "collision_mask"), &Precipitation::set_collision_mask);
	ObjectTypeDB::bind_method(_MD("get_collision_mask"), &Precipitation::get_collision_mask);

	ObjectTypeDB::bind_method(_MD("set_visibility_mask", "visibility_mask"), &Precipitation::set_visibility_mask);
	ObjectTypeDB::bind_method(_MD("get_visibility_mask"), &Precipitation::get_visibility_mask);

	ObjectTypeDB::bind_method(_MD("set_wind_velocity", "wind_velocity"), &Precipitation::set_wind_velocity);
	ObjectTypeDB::bind_method(_MD("get_wind_velocity"), &Precipitation::get_wind_velocity);
	ObjectTypeDB::bind_method(_MD("set_drop_particle_size", "drop_particle_size"), &Precipitation::set_drop_particle_size);
	ObjectTypeDB::bind_method(_MD("get_drop_particle_size"), &Precipitation::get_drop_particle_size);
	ObjectTypeDB::bind_method(_MD("set_using_collision", "using_collision"), &Precipitation::set_using_collision);
	ObjectTypeDB::bind_method(_MD("get_using_collision"), &Precipitation::get_using_collision);

	ObjectTypeDB::bind_method(_MD("set_percentage", "percentage"), &Precipitation::set_percentage);
	ObjectTypeDB::bind_method(_MD("get_percentage"), &Precipitation::get_percentage);

	ObjectTypeDB::bind_method(_MD("set_box_size", "box_size"), &Precipitation::set_box_size);
	ObjectTypeDB::bind_method(_MD("get_box_size"), &Precipitation::get_box_size);
	ObjectTypeDB::bind_method(_MD("set_drops_per_texture", "drops_per_texture"), &Precipitation::set_drops_per_texture);
	ObjectTypeDB::bind_method(_MD("get_drops_per_texture"), &Precipitation::get_drops_per_texture);
	ObjectTypeDB::bind_method(_MD("set_max_particles", "max_particles"), &Precipitation::set_max_particles);
	ObjectTypeDB::bind_method(_MD("get_max_particles"), &Precipitation::get_max_particles);
	ObjectTypeDB::bind_method(_MD("set_drop_particle_material", "drop_particle_material:Material"), &Precipitation::set_drop_particle_material);
	ObjectTypeDB::bind_method(_MD("get_drop_particle_material:Material"), &Precipitation::get_drop_particle_material);

	ObjectTypeDB::bind_method(_MD("set_min_speed", "min_speed"), &Precipitation::set_min_speed);
	ObjectTypeDB::bind_method(_MD("get_min_speed"), &Precipitation::get_min_speed);
	ObjectTypeDB::bind_method(_MD("set_max_speed", "max_speed"), &Precipitation::set_max_speed);
	ObjectTypeDB::bind_method(_MD("get_max_speed"), &Precipitation::get_max_speed);
	ObjectTypeDB::bind_method(_MD("set_min_mass", "min_mass"), &Precipitation::set_min_mass);
	ObjectTypeDB::bind_method(_MD("get_min_mass"), &Precipitation::get_min_mass);
	ObjectTypeDB::bind_method(_MD("set_max_mass", "max_mass"), &Precipitation::set_max_mass);
	ObjectTypeDB::bind_method(_MD("get_max_mass"), &Precipitation::get_max_mass);

	ObjectTypeDB::bind_method(_MD("set_using_billboards", "using_billboards"), &Precipitation::set_using_billboards);
	ObjectTypeDB::bind_method(_MD("get_using_billboards"), &Precipitation::get_using_billboards);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "camera", PROPERTY_HINT_NONE), _SCS("set_camera"), _SCS("get_camera"));

	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_ALL_FLAGS), _SCS("set_collision_mask"), _SCS("get_collision_mask"));
	ADD_PROPERTY(PropertyInfo(Variant::INT, "visibility_mask", PROPERTY_HINT_ALL_FLAGS), _SCS("set_visibility_mask"), _SCS("get_visibility_mask"));

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "wind_velocity", PROPERTY_HINT_NONE), _SCS("set_wind_velocity"), _SCS("get_wind_velocity"));
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "drop_particle_size", PROPERTY_HINT_NONE), _SCS("set_drop_particle_size"), _SCS("get_drop_particle_size"));
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "percentage", PROPERTY_HINT_NONE), _SCS("set_percentage"), _SCS("get_percentage"));
	ADD_PROPERTY(PropertyInfo(Variant::_AABB, "box_size", PROPERTY_HINT_NONE), _SCS("set_box_size"), _SCS("get_box_size"));
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_particles", PROPERTY_HINT_NONE), _SCS("set_max_particles"), _SCS("get_max_particles"));
	ADD_PROPERTY(PropertyInfo(Variant::INT, "drops_per_texture", PROPERTY_HINT_NONE), _SCS("set_drops_per_texture"), _SCS("get_drops_per_texture"));
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "drop_particle_material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), _SCS("set_drop_particle_material"), _SCS("get_drop_particle_material"));

	ADD_PROPERTY(PropertyInfo(Variant::REAL, "min_speed", PROPERTY_HINT_NONE), _SCS("set_min_speed"), _SCS("get_min_speed"));
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_speed", PROPERTY_HINT_NONE), _SCS("set_max_speed"), _SCS("get_max_speed"));
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "min_mass", PROPERTY_HINT_NONE), _SCS("set_min_mass"), _SCS("get_min_mass"));
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_mass", PROPERTY_HINT_NONE), _SCS("set_max_mass"), _SCS("get_max_mass"));

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "using_collision", PROPERTY_HINT_NONE), _SCS("set_using_collision"), _SCS("get_using_collision"));
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "using_billboards", PROPERTY_HINT_NONE), _SCS("set_using_billboards"), _SCS("get_using_billboards"));
}

Precipitation::Precipitation() {
	immediate_geometry = NULL;

	collision_mask = 1;
	visibility_mask = 1;
	wind_velocity = Vector3();

	drop_particle_size = 0.1;

	percentage = 1.0;
	box_size = Vector2(20, 10);
	max_particles = 1024;
	drops_per_texture = 1;

	min_speed = 0.15;
	max_speed = 0.2;
	min_mass = 0.075;
	max_mass = 0.085;

	using_collision = true;
	using_billboards = false;

	particle_head = NULL;
}

Precipitation::~Precipitation() {
}