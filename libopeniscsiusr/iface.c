/*
 * Copyright (C) 2017 Red Hat, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Gris Ge <fge@redhat.com>
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE			/* For NI_MAXHOST */
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <net/if.h>
#include <netdb.h>
#include <assert.h>
#include <inttypes.h>

#include "libopeniscsiusr/libopeniscsiusr.h"
#include "misc.h"
#include "sysfs.h"
#include "iface.h"

#define ISCSI_MAX_IFACE_LEN			65
#define ISCSI_TRANSPORT_NAME_MAXLEN		16
#define ISCSI_MAX_STR_LEN			80
#define ISCSI_HWADDRESS_BUF_SIZE		18
#define TARGET_NAME_MAXLEN			255
/* ^ TODO(Gris Ge): Above 5 constants are copy from usr/config.h, need to
 *		    verify them in linux kernel code
 */

#define DEFAULT_IFACENAME	"default"
#define DEFAULT_NETDEV		"default"
#define DEFAULT_IPADDRESS	"default"
#define DEFAULT_HWADDRESS	"default"


/* Just copy from `struct iface_rec` from usr/config.h */
struct iscsi_iface {
	/* iscsi iface record name */
	char			name[ISCSI_MAX_IFACE_LEN];
	uint32_t		iface_num;
	/* network layer iface name (eth0) */
	char			netdev[IFNAMSIZ];
	char			ipaddress[NI_MAXHOST];
	char			subnet_mask[NI_MAXHOST];
	char			gateway[NI_MAXHOST];
	char			bootproto[ISCSI_MAX_STR_LEN];
	char			ipv6_linklocal[NI_MAXHOST];
	char			ipv6_router[NI_MAXHOST];
	char			ipv6_autocfg[NI_MAXHOST];
	char			linklocal_autocfg[NI_MAXHOST];
	char			router_autocfg[NI_MAXHOST];
	uint16_t		vlan_id;
	uint8_t			vlan_priority;
	char			vlan_state[ISCSI_MAX_STR_LEN];
	char			state[ISCSI_MAX_STR_LEN]; /* 0 = disable,
							   * 1 = enable */
	uint16_t		mtu;
	uint16_t		port;
	char			delayed_ack[ISCSI_MAX_STR_LEN];
	char			nagle[ISCSI_MAX_STR_LEN];
	char			tcp_wsf_state[ISCSI_MAX_STR_LEN];
	uint8_t			tcp_wsf;
	uint8_t			tcp_timer_scale;
	char			tcp_timestamp[ISCSI_MAX_STR_LEN];
	char			dhcp_dns[ISCSI_MAX_STR_LEN];
	char			dhcp_slp_da[ISCSI_MAX_STR_LEN];
	char			tos_state[ISCSI_MAX_STR_LEN];
	uint8_t			tos;
	char			gratuitous_arp[ISCSI_MAX_STR_LEN];
	char			dhcp_alt_client_id_state[ISCSI_MAX_STR_LEN];
	char			dhcp_alt_client_id[ISCSI_MAX_STR_LEN];
	char			dhcp_req_vendor_id_state[ISCSI_MAX_STR_LEN];
	char			dhcp_vendor_id_state[ISCSI_MAX_STR_LEN];
	char			dhcp_vendor_id[ISCSI_MAX_STR_LEN];
	char			dhcp_learn_iqn[ISCSI_MAX_STR_LEN];
	char			fragmentation[ISCSI_MAX_STR_LEN];
	char			incoming_forwarding[ISCSI_MAX_STR_LEN];
	uint8_t			ttl;
	char			gratuitous_neighbor_adv[ISCSI_MAX_STR_LEN];
	char			redirect[ISCSI_MAX_STR_LEN];
	char			mld[ISCSI_MAX_STR_LEN];
	uint32_t		flow_label;
	uint32_t		traffic_class;
	uint8_t			hop_limit;
	uint32_t		nd_reachable_tmo;
	uint32_t		nd_rexmit_time;
	uint32_t		nd_stale_tmo;
	uint8_t			dup_addr_detect_cnt;
	uint32_t		router_adv_link_mtu;
	uint16_t		def_task_mgmt_tmo;
	char			header_digest[ISCSI_MAX_STR_LEN];
	char			data_digest[ISCSI_MAX_STR_LEN];
	char			immediate_data[ISCSI_MAX_STR_LEN];
	char			initial_r2t[ISCSI_MAX_STR_LEN];
	char			data_seq_inorder[ISCSI_MAX_STR_LEN];
	char			data_pdu_inorder[ISCSI_MAX_STR_LEN];
	uint8_t			erl;
	uint32_t		max_recv_dlength;
	uint32_t		first_burst_len;
	uint16_t		max_out_r2t;
	uint32_t		max_burst_len;
	char			chap_auth[ISCSI_MAX_STR_LEN];
	char			bidi_chap[ISCSI_MAX_STR_LEN];
	char			strict_login_comp[ISCSI_MAX_STR_LEN];
	char			discovery_auth[ISCSI_MAX_STR_LEN];
	char			discovery_logout[ISCSI_MAX_STR_LEN];
	char			port_state[ISCSI_MAX_STR_LEN];
	char			port_speed[ISCSI_MAX_STR_LEN];
	/*
	 * TODO: we may have to make this bigger and interconnect
	 * specific for infiniband
	 */
	char			hwaddress[ISCSI_HWADDRESS_BUF_SIZE];
	char			transport_name[ISCSI_TRANSPORT_NAME_MAXLEN];
	/*
	 * This is only used for boot now, but the iser guys
	 * can use this for their virtualization idea.
	 */
	char			alias[TARGET_NAME_MAXLEN + 1];
	char			iname[TARGET_NAME_MAXLEN + 1];
};

_iscsi_getter_func_gen(iscsi_iface, hwaddress, const char *);
_iscsi_getter_func_gen(iscsi_iface, transport_name, const char *);
_iscsi_getter_func_gen(iscsi_iface, ipaddress, const char *);
_iscsi_getter_func_gen(iscsi_iface, netdev, const char *);
_iscsi_getter_func_gen(iscsi_iface, iname, const char *);
_iscsi_getter_func_gen(iscsi_iface, port_state, const char *);
_iscsi_getter_func_gen(iscsi_iface, port_speed, const char *);
_iscsi_getter_func_gen(iscsi_iface, name, const char *);

int _iscsi_iface_get(struct iscsi_context *ctx, uint32_t host_id, uint32_t sid,
		     const char *iface_kern_id, struct iscsi_iface **iface)
{
	int rc = LIBISCSI_OK;
	char sysfs_se_dir_path[PATH_MAX];
	char sysfs_sh_dir_path[PATH_MAX];
	char sysfs_scsi_host_dir_path[PATH_MAX];
	char sysfs_iface_dir_path[PATH_MAX];
	char proc_name[ISCSI_TRANSPORT_NAME_MAXLEN];

	assert(ctx != NULL);
	assert(host_id != 0);
	assert(sid != 0);
	/* TODO(Gris Ge): Handle when sid == 0(ignored) */
	assert(iface != NULL);

	*iface = NULL;

	*iface = (struct iscsi_iface *) malloc(sizeof(struct iscsi_iface));
	_alloc_null_check(ctx, *iface, rc, out);

	snprintf(sysfs_se_dir_path, PATH_MAX, "%s/session%" PRIu32,
		 _ISCSI_SYS_SESSION_DIR, sid);
	snprintf(sysfs_sh_dir_path, PATH_MAX, "%s/host%" PRIu32,
		 _ISCSI_SYS_HOST_DIR, host_id);
	snprintf(sysfs_scsi_host_dir_path, PATH_MAX, "%s/host%" PRIu32,
		 _SCSI_SYS_HOST_DIR, host_id);
	if (iface_kern_id != NULL)
		snprintf(sysfs_iface_dir_path, PATH_MAX, "%s/%s",
			 _ISCSI_SYS_IFACE_DIR, iface_kern_id);

	_good(_sysfs_prop_get_str(ctx, sysfs_scsi_host_dir_path, "proc_name",
				  proc_name, sizeof(proc_name) / sizeof(char),
				  NULL /* raise error if failed */),
	      rc, out);

	if (strncmp(proc_name, "iscsi_", strlen("iscsi_")) == 0)
		strncpy((*iface)->transport_name, proc_name + strlen("iscsi_"),
			sizeof((*iface)->transport_name) / sizeof(char));
	else
		strncpy((*iface)->transport_name, proc_name,
			sizeof((*iface)->transport_name) / sizeof(char));

	_good(_sysfs_prop_get_str(ctx, sysfs_sh_dir_path, "hwaddress",
				  (*iface)->hwaddress,
				  sizeof((*iface)->hwaddress) / sizeof(char),
				  DEFAULT_HWADDRESS),
	      rc, out);

	if (iface_kern_id != NULL)
		_good(_sysfs_prop_get_str(ctx, sysfs_iface_dir_path,
					  "ipaddress",
					  (*iface)->ipaddress,
					  sizeof((*iface)->ipaddress) /
					  sizeof(char), DEFAULT_IPADDRESS),
		      rc, out);
	else
		_good(_sysfs_prop_get_str(ctx, sysfs_sh_dir_path, "ipaddress",
					(*iface)->ipaddress,
					sizeof((*iface)->ipaddress) /
					sizeof(char), DEFAULT_IPADDRESS),
		      rc, out);

	_good(_sysfs_prop_get_str(ctx, sysfs_sh_dir_path, "netdev",
				  (*iface)->netdev,
				  sizeof((*iface)->netdev) / sizeof(char),
				  DEFAULT_NETDEV),
	      rc, out);

	if (_sysfs_prop_get_str(NULL /* Ignore error */, sysfs_se_dir_path,
				  "initiatorname",
				  (*iface)->iname,
				  sizeof((*iface)->iname) / sizeof(char),
				  "")
	    != LIBISCSI_OK) {
		_debug(ctx, "Failed to read initiatorname from %s folder",
		       sysfs_se_dir_path);
		_good(_sysfs_prop_get_str(ctx, sysfs_sh_dir_path,
					  "initiatorname", (*iface)->iname,
					  sizeof((*iface)->iname) /
					  sizeof(char), ""),
		      rc, out);
	}

	_good(_sysfs_prop_get_str(ctx, sysfs_sh_dir_path, "port_state",
				  (*iface)->port_state,
				  sizeof((*iface)->port_state) / sizeof(char),
				  "unknown"),
	      rc, out);

	if (strcmp((*iface)->port_state, "Unknown!") == 0)
		strncpy((*iface)->port_state, "unknown",
			sizeof((*iface)->port_state) / sizeof(char));

	_good(_sysfs_prop_get_str(ctx, sysfs_sh_dir_path, "port_speed",
				  (*iface)->port_speed,
				  sizeof((*iface)->port_speed) / sizeof(char),
				  "unknown"),
	      rc, out);

	if (strncmp((*iface)->port_speed, "Unknown", strlen("Unknown")) == 0)
		strncpy((*iface)->port_speed, "unknown",
			sizeof((*iface)->port_speed) / sizeof(char));

	_good(_sysfs_prop_get_str(NULL /* Ignore error */, sysfs_se_dir_path,
				  "ifacename",
				  (*iface)->name,
				  sizeof((*iface)->name)/sizeof(char),
				  ""),
	      rc, out);
	if (strcmp((*iface)->name, "") == 0) {
		_debug(ctx, "Failed to query ifacename from %s folder",
		       sysfs_se_dir_path);
		/*
		 * if the ifacename file is not there then we are
		 * using a older kernel and can try to find the
		 * binding by the net info which was used on these
		 * older kernels.
		 */

		/*TODO(Gris Ge): need to parse /etc/iscsi/ifaces/<iface_name>
		 * files to find a match. I will add the code later when
		 * we expose more defiled information on iscsi_iface.
		 */
		strncpy((*iface)->name, DEFAULT_IFACENAME,
			sizeof((*iface)->name) / sizeof(char));
	}

out:
	if (rc != LIBISCSI_OK) {
		_iscsi_iface_free(*iface);
		*iface = NULL;
	}
	return rc;
}

void _iscsi_iface_free(struct iscsi_iface *iface)
{
	free(iface);
}
