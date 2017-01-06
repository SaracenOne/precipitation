#include "register_types.h"
#ifndef _3D_DISABLED
#include "object_type_db.h"
#endif
#include "precipitation.h"

void register_precipitation_types() {
#ifndef _3D_DISABLED
	ObjectTypeDB::register_type<Precipitation>();
#endif
}
void unregister_precipitation_types() {
	return;
}
