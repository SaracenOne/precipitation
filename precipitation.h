#ifndef PRECIPITATION_H
#define PRECIPITATION_H

#include "scene/3d/spatial.h"
#include "scene/3d/camera.h"
#include "scene/3d/immediate_geometry.h"

// TODO: create a new render primitive which calls into a custom rendering method every camera pass to allow this
// effect to be rendered differently for every camera/viewport.

class Precipitation : public Spatial {

	OBJ_TYPE(Precipitation,Spatial);
	OBJ_SAVE_TYPE(Precipitation);
protected:
	class Particle {
	public:
		Vector3 position;
		Vector3 render_position;
		Vector3 hit_pos;
		float velocity;
		float time;
		float mass;
		float anim_start_time;
		uint32_t tex_coord_index;

		bool is_valid;
		bool to_render;
		Particle *next = NULL;
	};

	NodePath camera_path = NodePath();

	uint32_t collision_mask;
	uint32_t visibility_mask;
	Vector3 wind_velocity;

	Ref<Material> drop_particle_material;
	float drop_particle_size;

	float percentage;
	Vector2 box_size;
	int max_particles;
	int drops_per_texture;

	float min_speed;
	float max_speed;
	float min_mass;
	float max_mass;

	bool using_collision;
	bool using_billboards;

//
	ImmediateGeometry *immediate_geometry = NULL;

	AABB visibility_box;
	Ref<World> w3d = NULL;
	PhysicsDirectSpaceState *dss = NULL;
	Camera *camera_node = NULL;

	bool pending_update = false;
	Particle *particle_head;
	Vector<Vector2> cached_coordinates;

public:
	void calculate_particle_cutoff_point(Precipitation::Particle *p_particle, const AABB &p_box, const Vector3 &p_wind_velocity);
	void spawn_particle(Precipitation::Particle *p_particle, const float p_delta);
	void spawn_new_particle(Precipitation::Particle *p_particle, const float p_delta);
	void empty_particles();
	void populate_particles(const float p_delta);

	void wrap_particle(Particle *p_particle, AABB &p_box, Vector3 &p_wind_velocity, const float p_delta);
	void update_render_cache();
	void draw_particles();
	void _precipitation_process(const float p_delta);

	_FORCE_INLINE_ void set_camera(const NodePath &p_nodepath) {
		camera_path = p_nodepath;
		if (has_node(camera_path)) {
			camera_node = static_cast<Camera *>(get_node(camera_path));
		}
		else {
			camera_node = NULL;
		}
	}

	_FORCE_INLINE_ NodePath get_camera() const {
		return camera_path;
	}

	_FORCE_INLINE_ void set_collision_mask(const int p_collision_mask) {
		collision_mask = p_collision_mask;
	}

	_FORCE_INLINE_ int get_collision_mask() const {
		return collision_mask;
	}

	_FORCE_INLINE_ void set_visibility_mask(const int p_visibility_mask) {
		visibility_mask = p_visibility_mask;
		if (immediate_geometry != NULL) {
			immediate_geometry->set_layer_mask(visibility_mask);
		}
	}

	_FORCE_INLINE_ int get_visibility_mask() const {
		return visibility_mask;
	}

	_FORCE_INLINE_ void set_wind_velocity(const Vector3 p_wind_velocity) {
		wind_velocity = p_wind_velocity;
	}

	_FORCE_INLINE_ Vector3 get_wind_velocity() const {
		return wind_velocity;
	}

	_FORCE_INLINE_ void set_drop_particle_size(const float p_drop_particle_size) {
		drop_particle_size = p_drop_particle_size;
	}

	_FORCE_INLINE_ float get_drop_particle_size() const {
		return drop_particle_size;
	}

	_FORCE_INLINE_ void set_percentage(const float p_percentage) {
		percentage = p_percentage;
	}

	_FORCE_INLINE_ float get_percentage() const {
		return percentage;
	}

	_FORCE_INLINE_ void set_box_size(const Vector2 p_box_size) {
		box_size = p_box_size;
	}

	_FORCE_INLINE_ Vector2 get_box_size() const {
		return box_size;
	}

	_FORCE_INLINE_ void set_max_particles(const int p_max_particles) {
		max_particles = p_max_particles;
		pending_update = true;
	}

	_FORCE_INLINE_ int get_max_particles() const {
		return max_particles;
	}

	_FORCE_INLINE_ void set_drops_per_texture(const int p_dps) {
		drops_per_texture = p_dps;
		pending_update = true;
	}

	_FORCE_INLINE_ int get_drops_per_texture() const {
		return drops_per_texture;
	}

	_FORCE_INLINE_ void set_drop_particle_material(Ref<Material> p_material) {
		drop_particle_material = p_material;
		if (immediate_geometry != NULL)
			immediate_geometry->set_material_override(drop_particle_material);
	}

	_FORCE_INLINE_ Ref<Material>get_drop_particle_material() const {
		return drop_particle_material;
	}

	_FORCE_INLINE_ void set_min_speed(const float p_min_speed) {
		min_speed = p_min_speed;
	}

	_FORCE_INLINE_ float get_min_speed() const {
		return min_speed;
	}

	_FORCE_INLINE_ void set_max_speed(const float p_max_speed) {
		max_speed = p_max_speed;
	}

	_FORCE_INLINE_ float get_max_speed() const {
		return max_speed;
	}

	_FORCE_INLINE_ void set_min_mass(const float p_min_mass) {
		min_mass = p_min_mass;
	}

	_FORCE_INLINE_ float get_min_mass() const {
		return min_mass;
	}

	_FORCE_INLINE_ void set_max_mass(const float p_max_mass) {
		max_mass = p_max_mass;
	}

	_FORCE_INLINE_ float get_max_mass() const {
		return max_mass;
	}
	
	_FORCE_INLINE_ void set_using_collision(const bool p_using_collision) {
		using_collision = p_using_collision;
	}

	_FORCE_INLINE_ bool get_using_collision() const {
		return using_collision;
	}

	_FORCE_INLINE_ void set_using_billboards(const bool p_using_billboards) {
		using_billboards = p_using_billboards;
	}

	_FORCE_INLINE_ bool get_using_billboards() const {
		return using_billboards;
	}

	void _notification(int p_what);
	static void _bind_methods();
public:
	Precipitation();
	~Precipitation();
};


#endif // PRECIPITATION_H