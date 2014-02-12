/*
 * wicked dhcp4 supplicant
 *
 * Copyright (C) 2010-2012 Olaf Kirch <okir@suse.de>
 */

#ifndef __WICKED_DHCP4_PRIVATE_H__
#define __WICKED_DHCP4_PRIVATE_H__


#include <sys/time.h>
#include <wicked/netinfo.h>
#include <wicked/addrconf.h>
#include <wicked/wicked.h>
#include <wicked/socket.h>
#include "netinfo_priv.h"
#include "buffer.h"

enum {
	NI_DHCP4_STATE_INIT,
	NI_DHCP4_STATE_SELECTING,
	NI_DHCP4_STATE_REQUESTING,
	NI_DHCP4_STATE_VALIDATING,
	NI_DHCP4_STATE_BOUND,
	NI_DHCP4_STATE_RENEWING,
	NI_DHCP4_STATE_REBINDING,
	NI_DHCP4_STATE_REBOOT,
	NI_DHCP4_STATE_RELEASED,

	__NI_DHCP4_STATE_MAX,
};

typedef struct ni_dhcp4_message ni_dhcp4_message_t;
typedef struct ni_dhcp4_config ni_dhcp4_config_t;
typedef struct ni_dhcp4_request	ni_dhcp4_request_t;

typedef struct ni_dhcp4_device {
	struct ni_dhcp4_device *	next;
	unsigned int		users;

	char *			ifname;
	ni_linkinfo_t		link;

	struct {
	    int			state;
	    const ni_timer_t *	timer;
	} fsm;

	ni_capture_devinfo_t	system;

	time_t			start_time;	/* when we starting managing */

	ni_dhcp4_request_t *	request;
	ni_dhcp4_config_t *	config;
	ni_addrconf_lease_t *	lease;

	ni_capture_t *		capture;
	int			listen_fd;	/* for DHCP4 only */

	unsigned int		failed : 1,
				notify : 1;

	struct {
	    uint32_t		xid;
	    unsigned int	nak_backoff;	/* backoff timer when we get NAKs */
	    unsigned int	accept_any_offer : 1;
	} dhcp4;

	ni_buffer_t		message;

	struct {
	   ni_arp_socket_t *	handle;
	   unsigned int		nprobes;
	   unsigned int		nclaims;
	} arp;

	struct {
	   ni_addrconf_lease_t *lease;
	   int			weight;
	} best_offer;
} ni_dhcp4_device_t;

#define NI_DHCP4_RESEND_TIMEOUT_INIT	3	/* seconds */
#define NI_DHCP4_RESEND_TIMEOUT_MAX	60	/* seconds */
#define NI_DHCP4_REQUEST_TIMEOUT		60	/* seconds */
#define NI_DHCP4_ARP_TIMEOUT		200	/* msec */

/* Initial discovery period while we scan all available leases. */
#define NI_DHCP4_DISCOVERY_TIMEOUT	20	/* seconds */

enum {
	DHCP4_DO_ARP		= 0x00000001, /* TODO: -> wickedd */
	DHCP4_DO_HOSTNAME	= 0x00000002,
	DHCP4_DO_RESOLVER	= 0x00000004,
	DHCP4_DO_NIS		= 0x00000008,
	DHCP4_DO_NTP		= 0x00000010,
	DHCP4_DO_CSR		= 0x00000020,
	DHCP4_DO_MSCSR		= 0x00000040,
	DHCP4_DO_GATEWAY	= 0x00000080,
	DHCP4_DO_ROOT		= 0x00000100,
	DHCP4_DO_NDS		= 0x00000200,
	DHCP4_DO_SMB		= 0x00000400,
	DHCP4_DO_SIP		= 0x00000800,
	DHCP4_DO_LPR		= 0x00001000,
	DHCP4_DO_LOG		= 0x00002000,
	DHCP4_DO_POSIX_TZ	= 0x00004000,
	DHCP4_DO_DEFAULT	= DHCP4_DO_ARP|DHCP4_DO_HOSTNAME|DHCP4_DO_RESOLVER|
				  DHCP4_DO_NIS|DHCP4_DO_NTP|DHCP4_DO_CSR|
				  DHCP4_DO_GATEWAY|DHCP4_DO_ROOT|
				  DHCP4_DO_NDS|DHCP4_DO_SMB|DHCP4_DO_SIP|
				  DHCP4_DO_LPR|DHCP4_DO_LOG|DHCP4_DO_POSIX_TZ
};

/*
 * This is the on-the wire request we receive from clients.
 */
struct ni_dhcp4_request {
	ni_uuid_t		uuid;
	ni_bool_t		enabled;

	ni_bool_t		dry_run;	/* discover but don't request + commit */
	unsigned int		settle_timeout;	/* wait that long before starting DHCP4 */
	unsigned int		acquire_timeout;/* how long we try before we give up */

	/* Options controlling what to put into the lease request */
	char *			hostname;
	char *			clientid;
	char *			vendor_class;
	unsigned int		lease_time;

	unsigned int		route_priority;

	/* Options what to update based on the info received from
	 * the DHCP4 server.
	 * This is a bitmap; individual bits correspond to
	 * NI_ADDRCONF_UPDATE_* (this is an index enum, not a bitmask) */
	unsigned int		update;
};

/*
 * This is what we turn the above ni_dhcp4_request_t into for
 * internal use.
 */
struct ni_dhcp4_config {
	ni_uuid_t		uuid;
	ni_bool_t		dry_run;

	/* A combination of DHCP4_DO_* flags above */
	unsigned int		flags;

	char			hostname[256];
	char			classid[48];
	int			fqdn;

	ni_opaque_t		client_id;
	ni_opaque_t		userclass;

	unsigned int		initial_discovery_timeout;
	unsigned int		request_timeout;
	unsigned int		resend_timeout;
	unsigned int		max_lease_time;
	unsigned int		update;

	unsigned int		route_priority;
};

enum ni_dhcp4_event {
	NI_DHCP4_EVENT_ACQUIRED =NI_EVENT_LEASE_ACQUIRED,
	NI_DHCP4_EVENT_RELEASED =NI_EVENT_LEASE_RELEASED,
	NI_DHCP4_EVENT_LOST =	NI_EVENT_LEASE_LOST
};

typedef void		ni_dhcp4_event_handler_t(enum ni_dhcp4_event event,
					const ni_dhcp4_device_t *dev,
					ni_addrconf_lease_t *lease);

extern ni_dhcp4_device_t *ni_dhcp4_active;

extern void		ni_dhcp4_set_event_handler(ni_dhcp4_event_handler_t);

extern int		ni_dhcp4_acquire(ni_dhcp4_device_t *, const ni_dhcp4_request_t *);
extern int		ni_dhcp4_release(ni_dhcp4_device_t *, const ni_uuid_t *);
extern void		ni_dhcp4_restart_leases(void);

extern int		ni_dhcp4_fsm_discover(ni_dhcp4_device_t *);
extern int		ni_dhcp4_fsm_release(ni_dhcp4_device_t *);
extern int		ni_dhcp4_fsm_process_dhcp4_packet(ni_dhcp4_device_t *, ni_buffer_t *);
extern int		ni_dhcp4_fsm_commit_lease(ni_dhcp4_device_t *, ni_addrconf_lease_t *);
extern int		ni_dhcp4_fsm_recover_lease(ni_dhcp4_device_t *, const ni_dhcp4_request_t *);
extern int		ni_dhcp4_build_message(const ni_dhcp4_device_t *,
				unsigned int, const ni_addrconf_lease_t *, ni_buffer_t *);
extern void		ni_dhcp4_fsm_link_up(ni_dhcp4_device_t *);
extern void		ni_dhcp4_fsm_link_down(ni_dhcp4_device_t *);

extern int		ni_dhcp4_parse_response(const ni_dhcp4_message_t *, ni_buffer_t *, ni_addrconf_lease_t **);

extern int		ni_dhcp4_socket_open(ni_dhcp4_device_t *);

extern int		ni_dhcp4_device_start(ni_dhcp4_device_t *);
extern void		ni_dhcp4_device_stop(ni_dhcp4_device_t *);
extern unsigned int	ni_dhcp4_device_uptime(const ni_dhcp4_device_t *, unsigned int);
extern ni_dhcp4_device_t *ni_dhcp4_device_new(const char *, const ni_linkinfo_t *);
extern ni_dhcp4_device_t *ni_dhcp4_device_by_index(unsigned int);
extern ni_dhcp4_device_t *ni_dhcp4_device_get(ni_dhcp4_device_t *);
extern void		ni_dhcp4_device_put(ni_dhcp4_device_t *);
extern void		ni_dhcp4_device_event(ni_dhcp4_device_t *, ni_netdev_t *, ni_event_t);
extern int		ni_dhcp4_device_reconfigure(ni_dhcp4_device_t *, const ni_netdev_t *);
extern void		ni_dhcp4_device_set_config(ni_dhcp4_device_t *, ni_dhcp4_config_t *);
extern void		ni_dhcp4_device_set_request(ni_dhcp4_device_t *, ni_dhcp4_request_t *);
extern void		ni_dhcp4_device_set_lease(ni_dhcp4_device_t *, ni_addrconf_lease_t *);
extern void		ni_dhcp4_device_drop_lease(ni_dhcp4_device_t *);
extern void		ni_dhcp4_device_alloc_buffer(ni_dhcp4_device_t *);
extern void		ni_dhcp4_device_drop_buffer(ni_dhcp4_device_t *);
extern int		ni_dhcp4_device_send_message(ni_dhcp4_device_t *, unsigned int, const ni_addrconf_lease_t *);
extern int		ni_dhcp4_device_send_message_unicast(ni_dhcp4_device_t *,
				unsigned int, const ni_addrconf_lease_t *);
extern void		ni_dhcp4_device_arm_retransmit(ni_dhcp4_device_t *dev);
extern void		ni_dhcp4_device_disarm_retransmit(ni_dhcp4_device_t *dev);
extern void		ni_dhcp4_device_retransmit(ni_dhcp4_device_t *);
extern void		ni_dhcp4_device_force_retransmit(ni_dhcp4_device_t *, unsigned int);
extern void		ni_dhcp4_device_arp_close(ni_dhcp4_device_t *);
extern void		ni_dhcp4_parse_client_id(ni_opaque_t *, unsigned short, const char *);
extern void		ni_dhcp4_set_client_id(ni_opaque_t *, const ni_hwaddr_t *);
extern void		ni_dhcp4_device_drop_best_offer(ni_dhcp4_device_t *);

extern int		ni_dhcp4_xml_from_lease(const ni_addrconf_lease_t *, xml_node_t *);
extern int		ni_dhcp4_xml_to_lease(ni_addrconf_lease_t *, const xml_node_t *);

extern const char *	ni_dhcp4_config_vendor_class(void);
extern int		ni_dhcp4_config_ignore_server(struct in_addr);
extern int		ni_dhcp4_config_have_server_preference(void);
extern int		ni_dhcp4_config_server_preference(struct in_addr);
extern unsigned int	ni_dhcp4_config_max_lease_time(void);
extern void		ni_dhcp4_config_free(ni_dhcp4_config_t *);

extern ni_dhcp4_request_t *ni_dhcp4_request_new(void);
extern void		ni_dhcp4_request_free(ni_dhcp4_request_t *);

extern void		ni_objectmodel_dhcp4_init(void);

#endif /* __WICKED_DHCP4_PRIVATE_H__ */
