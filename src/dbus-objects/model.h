#ifndef __WICKED_OBJECTMODEL_H__
#define __WICKED_OBJECTMODEL_H__

#define __NI_DBUS_PROPERTY_RO(fstem, __name)	NULL
#define __NI_DBUS_PROPERTY_ROP(fstem, __name)	__NI_DBUS_PROPERTY_PARSE_FN(fstem, __name)
#define __NI_DBUS_PROPERTY_RW(fstem, __name)	__NI_DBUS_PROPERTY_UPDATE_FN(fstem, __name)
#define __NI_DBUS_PROPERTY_RWP(fstem, __name) \
	__NI_DBUS_PROPERTY_PARSE_FN(fstem, __name), \
	__NI_DBUS_PROPERTY_UPDATE_FN(fstem, __name)
#define __NI_DBUS_PROPERTY_UPDATE_FN(fstem, __name) \
	.update = fstem ## _update_ ## __name
#define __NI_DBUS_PROPERTY_PARSE_FN(fstem, __name) \
	.parse = fstem ## _parse_ ## __name

#define __NI_DBUS_DUMMY_PROPERTY(__signature, __name) { \
	.name = #__name, \
	.signature = __signature, \
}
#define NI_DBUS_DUMMY_PROPERTY(type, __name) \
	__NI_DBUS_DUMMY_PROPERTY(DBUS_TYPE_##type##_AS_STRING, __name)
#define __NI_DBUS_PROPERTY(__signature, __name, __id, fstem, rw) { \
	.name = #__name, \
	.id = __id, \
	.signature = __signature, \
	.get = fstem ## _get_ ## __name, \
	.set = fstem ## _set_ ## __name, \
	__NI_DBUS_PROPERTY_##rw(fstem, __name), \
}
#define NI_DBUS_PROPERTY(type, __name, __id, fstem, rw) \
	__NI_DBUS_PROPERTY(DBUS_TYPE_##type##_AS_STRING, __name, __id, fstem, rw)

#define __pointer(base, offset_ptr) \
	((typeof(offset_ptr)) (((caddr_t) base) + (unsigned long) offset_ptr))

static inline dbus_bool_t
__ni_objectmodel_set_property_int(void *handle, int *member_offset, const ni_dbus_variant_t *result)
{
	if (handle == NULL)
		return FALSE;

	return ni_dbus_variant_get_int(result, __pointer(handle, member_offset));
}

static inline dbus_bool_t
__ni_objectmodel_set_property_uint(void *handle, unsigned int *member_offset, const ni_dbus_variant_t *result)
{
	if (handle == NULL)
		return FALSE;

	return ni_dbus_variant_get_uint(result, __pointer(handle, member_offset));
}

static inline dbus_bool_t
__ni_objectmodel_set_property_long(void *handle, long *member_offset, const ni_dbus_variant_t *result)
{
	if (handle == NULL)
		return FALSE;

	return ni_dbus_variant_get_long(result, __pointer(handle, member_offset));
}

static inline dbus_bool_t
__ni_objectmodel_set_property_ulong(void *handle, unsigned long *member_offset, const ni_dbus_variant_t *result)
{
	if (handle == NULL)
		return FALSE;

	return ni_dbus_variant_get_ulong(result, __pointer(handle, member_offset));
}

static inline dbus_bool_t
__ni_objectmodel_get_property_int(const void *handle, int *member_offset, ni_dbus_variant_t *result)
{
	if (handle == NULL)
		return FALSE;

	return ni_dbus_variant_set_int(result, *__pointer(handle, member_offset));
}

static inline dbus_bool_t
__ni_objectmodel_get_property_uint(const void *handle, unsigned int *member_offset, ni_dbus_variant_t *result)
{
	if (handle == NULL)
		return FALSE;

	return ni_dbus_variant_set_uint(result, *__pointer(handle, member_offset));
}

static inline dbus_bool_t
__ni_objectmodel_get_property_long(const void *handle, long *member_offset, ni_dbus_variant_t *result)
{
	if (handle == NULL)
		return FALSE;

	return ni_dbus_variant_set_long(result, *__pointer(handle, member_offset));
}

static inline dbus_bool_t
__ni_objectmodel_get_property_ulong(const void *handle, unsigned long *member_offset, ni_dbus_variant_t *result)
{
	if (handle == NULL)
		return FALSE;

	return ni_dbus_variant_set_ulong(result, *__pointer(handle, member_offset));
}


extern ni_dbus_service_t	wicked_dbus_ethernet_service;
extern ni_dbus_service_t	wicked_dbus_vlan_service;
extern ni_dbus_service_t	wicked_dbus_bridge_service;
extern ni_dbus_service_t	wicked_dbus_bond_service;
extern ni_dbus_service_t	wicked_dbus_bridge_port_dummy_service;
extern ni_dbus_service_t	wicked_dbus_bond_port_dummy_service;

extern ni_dbus_object_t *	ni_objectmodel_new_vlan(ni_dbus_server_t *server,
					const ni_dbus_object_t *config);
extern ni_dbus_object_t *	ni_objectmodel_new_bridge(ni_dbus_server_t *server,
					const ni_dbus_object_t *config);
extern ni_dbus_object_t *	ni_objectmodel_new_bond(ni_dbus_server_t *server,
					const ni_dbus_object_t *config);
extern ni_dbus_object_t *	ni_objectmodel_new_ppp(ni_dbus_server_t *server,
					const ni_dbus_object_t *config);

#endif /* __WICKED_OBJECTMODEL_H__ */
