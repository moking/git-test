// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
#include <limits.h>
#include <util/json.h>
#include <uuid/uuid.h>
#include <cxl/libcxl.h>
#include <json-c/json.h>
#include <json-c/printbuf.h>
#include <ccan/short_types/short_types.h>

#include "filter.h"
#include "json.h"

static struct json_object *util_cxl_memdev_health_to_json(
		struct cxl_memdev *memdev, unsigned long flags)
{
	struct json_object *jhealth;
	struct json_object *jobj;
	struct cxl_cmd *cmd;
	u32 field;
	int rc;

	jhealth = json_object_new_object();
	if (!jhealth)
		return NULL;
	if (!memdev)
		goto err_jobj;

	cmd = cxl_cmd_new_get_health_info(memdev);
	if (!cmd)
		goto err_jobj;

	rc = cxl_cmd_submit(cmd);
	if (rc < 0)
		goto err_cmd;
	rc = cxl_cmd_get_mbox_status(cmd);
	if (rc != 0)
		goto err_cmd;

	/* health_status fields */
	rc = cxl_cmd_health_info_get_maintenance_needed(cmd);
	jobj = json_object_new_boolean(rc);
	if (jobj)
		json_object_object_add(jhealth, "maintenance_needed", jobj);

	rc = cxl_cmd_health_info_get_performance_degraded(cmd);
	jobj = json_object_new_boolean(rc);
	if (jobj)
		json_object_object_add(jhealth, "performance_degraded", jobj);

	rc = cxl_cmd_health_info_get_hw_replacement_needed(cmd);
	jobj = json_object_new_boolean(rc);
	if (jobj)
		json_object_object_add(jhealth, "hw_replacement_needed", jobj);

	/* media_status fields */
	rc = cxl_cmd_health_info_get_media_normal(cmd);
	jobj = json_object_new_boolean(rc);
	if (jobj)
		json_object_object_add(jhealth, "media_normal", jobj);

	rc = cxl_cmd_health_info_get_media_not_ready(cmd);
	jobj = json_object_new_boolean(rc);
	if (jobj)
		json_object_object_add(jhealth, "media_not_ready", jobj);

	rc = cxl_cmd_health_info_get_media_persistence_lost(cmd);
	jobj = json_object_new_boolean(rc);
	if (jobj)
		json_object_object_add(jhealth, "media_persistence_lost", jobj);

	rc = cxl_cmd_health_info_get_media_data_lost(cmd);
	jobj = json_object_new_boolean(rc);
	if (jobj)
		json_object_object_add(jhealth, "media_data_lost", jobj);

	rc = cxl_cmd_health_info_get_media_powerloss_persistence_loss(cmd);
	jobj = json_object_new_boolean(rc);
	if (jobj)
		json_object_object_add(jhealth, "media_powerloss_persistence_loss", jobj);

	rc = cxl_cmd_health_info_get_media_shutdown_persistence_loss(cmd);
	jobj = json_object_new_boolean(rc);
	if (jobj)
		json_object_object_add(jhealth, "media_shutdown_persistence_loss", jobj);

	rc = cxl_cmd_health_info_get_media_persistence_loss_imminent(cmd);
	jobj = json_object_new_boolean(rc);
	if (jobj)
		json_object_object_add(jhealth, "media_persistence_loss_imminent", jobj);

	rc = cxl_cmd_health_info_get_media_powerloss_data_loss(cmd);
	jobj = json_object_new_boolean(rc);
	if (jobj)
		json_object_object_add(jhealth, "media_powerloss_data_loss", jobj);

	rc = cxl_cmd_health_info_get_media_shutdown_data_loss(cmd);
	jobj = json_object_new_boolean(rc);
	if (jobj)
		json_object_object_add(jhealth, "media_shutdown_data_loss", jobj);

	rc = cxl_cmd_health_info_get_media_data_loss_imminent(cmd);
	jobj = json_object_new_boolean(rc);
	if (jobj)
		json_object_object_add(jhealth, "media_data_loss_imminent", jobj);

	/* ext_status fields */
	if (cxl_cmd_health_info_get_ext_life_used_normal(cmd))
		jobj = json_object_new_string("normal");
	else if (cxl_cmd_health_info_get_ext_life_used_warning(cmd))
		jobj = json_object_new_string("warning");
	else if (cxl_cmd_health_info_get_ext_life_used_critical(cmd))
		jobj = json_object_new_string("critical");
	else
		jobj = json_object_new_string("unknown");
	if (jobj)
		json_object_object_add(jhealth, "ext_life_used", jobj);

	if (cxl_cmd_health_info_get_ext_temperature_normal(cmd))
		jobj = json_object_new_string("normal");
	else if (cxl_cmd_health_info_get_ext_temperature_warning(cmd))
		jobj = json_object_new_string("warning");
	else if (cxl_cmd_health_info_get_ext_temperature_critical(cmd))
		jobj = json_object_new_string("critical");
	else
		jobj = json_object_new_string("unknown");
	if (jobj)
		json_object_object_add(jhealth, "ext_temperature", jobj);

	if (cxl_cmd_health_info_get_ext_corrected_volatile_normal(cmd))
		jobj = json_object_new_string("normal");
	else if (cxl_cmd_health_info_get_ext_corrected_volatile_warning(cmd))
		jobj = json_object_new_string("warning");
	else
		jobj = json_object_new_string("unknown");
	if (jobj)
		json_object_object_add(jhealth, "ext_corrected_volatile", jobj);

	if (cxl_cmd_health_info_get_ext_corrected_persistent_normal(cmd))
		jobj = json_object_new_string("normal");
	else if (cxl_cmd_health_info_get_ext_corrected_persistent_warning(cmd))
		jobj = json_object_new_string("warning");
	else
		jobj = json_object_new_string("unknown");
	if (jobj)
		json_object_object_add(jhealth, "ext_corrected_persistent", jobj);

	/* other fields */
	field = cxl_cmd_health_info_get_life_used(cmd);
	if (field != 0xff) {
		jobj = json_object_new_int(field);
		if (jobj)
			json_object_object_add(jhealth, "life_used_percent", jobj);
	}

	field = cxl_cmd_health_info_get_temperature(cmd);
	if (field != 0xffff) {
		jobj = json_object_new_int(field);
		if (jobj)
			json_object_object_add(jhealth, "temperature", jobj);
	}

	field = cxl_cmd_health_info_get_dirty_shutdowns(cmd);
	jobj = util_json_new_u64(field);
	if (jobj)
		json_object_object_add(jhealth, "dirty_shutdowns", jobj);

	field = cxl_cmd_health_info_get_volatile_errors(cmd);
	jobj = util_json_new_u64(field);
	if (jobj)
		json_object_object_add(jhealth, "volatile_errors", jobj);

	field = cxl_cmd_health_info_get_pmem_errors(cmd);
	jobj = util_json_new_u64(field);
	if (jobj)
		json_object_object_add(jhealth, "pmem_errors", jobj);

	cxl_cmd_unref(cmd);
	return jhealth;

err_cmd:
	cxl_cmd_unref(cmd);
err_jobj:
	json_object_put(jhealth);
	return NULL;
}

struct json_object *util_cxl_memdev_to_json(struct cxl_memdev *memdev,
		unsigned long flags)
{
	const char *devname = cxl_memdev_get_devname(memdev);
	struct json_object *jdev, *jobj;
	unsigned long long serial;
	int numa_node;

	jdev = json_object_new_object();
	if (!jdev)
		return NULL;

	jobj = json_object_new_string(devname);
	if (jobj)
		json_object_object_add(jdev, "memdev", jobj);

	jobj = util_json_object_size(cxl_memdev_get_pmem_size(memdev), flags);
	if (jobj)
		json_object_object_add(jdev, "pmem_size", jobj);

	jobj = util_json_object_size(cxl_memdev_get_ram_size(memdev), flags);
	if (jobj)
		json_object_object_add(jdev, "ram_size", jobj);

	if (flags & UTIL_JSON_HEALTH) {
		jobj = util_cxl_memdev_health_to_json(memdev, flags);
		if (jobj)
			json_object_object_add(jdev, "health", jobj);
	}

	serial = cxl_memdev_get_serial(memdev);
	if (serial < ULLONG_MAX) {
		jobj = util_json_object_hex(serial, flags);
		if (jobj)
			json_object_object_add(jdev, "serial", jobj);
	}

	numa_node = cxl_memdev_get_numa_node(memdev);
	if (numa_node >= 0) {
		jobj = json_object_new_int(numa_node);
		if (jobj)
			json_object_object_add(jdev, "numa_node", jobj);
	}

	jobj = json_object_new_string(cxl_memdev_get_host(memdev));
	if (jobj)
		json_object_object_add(jdev, "host", jobj);

	if (!cxl_memdev_is_enabled(memdev)) {
		jobj = json_object_new_string("disabled");
		if (jobj)
			json_object_object_add(jdev, "state", jobj);
	}

	return jdev;
}

void util_cxl_dports_append_json(struct json_object *jport,
				 struct cxl_port *port, const char *ident,
				 const char *serial, unsigned long flags)
{
	struct json_object *jobj, *jdports;
	struct cxl_dport *dport;
	int val;

	val = cxl_port_get_nr_dports(port);
	if (!val || !(flags & UTIL_JSON_TARGETS))
		return;

	jobj = json_object_new_int(val);
	if (jobj)
		json_object_object_add(jport, "nr_dports", jobj);

	jdports = json_object_new_array();
	if (!jdports)
		return;

	cxl_dport_foreach(port, dport) {
		struct json_object *jdport;
		const char *phys_node;

		if (!util_cxl_dport_filter_by_memdev(dport, ident, serial))
			continue;

		jdport = json_object_new_object();
		if (!jdport)
			continue;

		jobj = json_object_new_string(cxl_dport_get_devname(dport));
		if (jobj)
			json_object_object_add(jdport, "dport", jobj);

		phys_node = cxl_dport_get_physical_node(dport);
		if (phys_node) {
			jobj = json_object_new_string(phys_node);
			if (jobj)
				json_object_object_add(jdport, "alias", jobj);
		}

		val = cxl_dport_get_id(dport);
		jobj = util_json_object_hex(val, flags);
		if (jobj)
			json_object_object_add(jdport, "id", jobj);

		json_object_array_add(jdports, jdport);
	}

	json_object_object_add(jport, "dports", jdports);
}

struct json_object *util_cxl_bus_to_json(struct cxl_bus *bus,
					 unsigned long flags)
{
	const char *devname = cxl_bus_get_devname(bus);
	struct json_object *jbus, *jobj;

	jbus = json_object_new_object();
	if (!jbus)
		return NULL;

	jobj = json_object_new_string(devname);
	if (jobj)
		json_object_object_add(jbus, "bus", jobj);

	jobj = json_object_new_string(cxl_bus_get_provider(bus));
	if (jobj)
		json_object_object_add(jbus, "provider", jobj);

	return jbus;
}

struct json_object *util_cxl_decoder_to_json(struct cxl_decoder *decoder,
					     unsigned long flags)
{
	const char *devname = cxl_decoder_get_devname(decoder);
	struct cxl_port *port = cxl_decoder_get_port(decoder);
	struct json_object *jdecoder, *jobj;
	u64 val;

	jdecoder = json_object_new_object();
	if (!jdecoder)
		return NULL;

	jobj = json_object_new_string(devname);
	if (jobj)
		json_object_object_add(jdecoder, "decoder", jobj);

	val = cxl_decoder_get_resource(decoder);
	if (val < ULLONG_MAX) {
		jobj = util_json_object_hex(val, flags);
		if (jobj)
			json_object_object_add(jdecoder, "resource", jobj);
	}

	val = cxl_decoder_get_size(decoder);
	if (val < ULLONG_MAX) {
		jobj = util_json_object_size(val, flags);
		if (jobj)
			json_object_object_add(jdecoder, "size", jobj);
	}

	if (val == 0) {
		jobj = json_object_new_string("disabled");
		if (jobj)
			json_object_object_add(jdecoder, "state", jobj);
	}

	if (cxl_port_is_root(port) && cxl_decoder_is_mem_capable(decoder)) {
		if (cxl_decoder_is_pmem_capable(decoder)) {
			jobj = json_object_new_boolean(true);
			if (jobj)
				json_object_object_add(jdecoder, "pmem_capable",
						       jobj);
		}
		if (cxl_decoder_is_volatile_capable(decoder)) {
			jobj = json_object_new_boolean(true);
			if (jobj)
				json_object_object_add(
					jdecoder, "volatile_capable", jobj);
		}
	}

	if (cxl_port_is_root(port) &&
	    cxl_decoder_is_accelmem_capable(decoder)) {
		jobj = json_object_new_boolean(true);
		if (jobj)
			json_object_object_add(jdecoder, "accelmem_capable",
					       jobj);
	}

	return jdecoder;
}

void util_cxl_targets_append_json(struct json_object *jdecoder,
				  struct cxl_decoder *decoder,
				  const char *ident, const char *serial,
				  unsigned long flags)
{
	struct cxl_port *port = cxl_decoder_get_port(decoder);
	struct json_object *jobj, *jtargets;
	struct cxl_target *target;
	int val;

	/* Endpoints don't have targets, they *are* targets */
	if (cxl_port_is_endpoint(port))
		return;

	val = cxl_decoder_get_nr_targets(decoder);
	jobj = json_object_new_int(val);
	if (jobj)
		json_object_object_add(jdecoder, "nr_targets", jobj);

	if (!(flags & UTIL_JSON_TARGETS) ||
	    !cxl_decoder_get_nr_targets(decoder))
		return;

	jtargets = json_object_new_array();
	if (!jtargets)
		return;

	cxl_target_foreach(decoder, target) {
		struct json_object *jtarget;
		const char *phys_node;
		const char *devname;

		if (!util_cxl_target_filter_by_memdev(target, ident, serial))
			continue;

		jtarget = json_object_new_object();
		if (!jtarget)
			continue;

		devname = cxl_target_get_devname(target);
		jobj = json_object_new_string(devname);
		if (jobj)
			json_object_object_add(jtarget, "target", jobj);

		phys_node = cxl_target_get_physical_node(target);
		if (phys_node) {
			jobj = json_object_new_string(phys_node);
			if (jobj)
				json_object_object_add(jtarget, "alias", jobj);
		}

		val = cxl_target_get_position(target);
		jobj = json_object_new_int(val);
		if (jobj)
			json_object_object_add(jtarget, "position", jobj);

		val = cxl_target_get_id(target);
		jobj = util_json_object_hex(val, flags);
		if (jobj)
			json_object_object_add(jtarget, "id", jobj);

		json_object_array_add(jtargets, jtarget);
	}

	json_object_object_add(jdecoder, "targets", jtargets);
}

static struct json_object *__util_cxl_port_to_json(struct cxl_port *port,
						   const char *name_key,
						   unsigned long flags)
{
	const char *devname = cxl_port_get_devname(port);
	struct json_object *jport, *jobj;

	jport = json_object_new_object();
	if (!jport)
		return NULL;

	jobj = json_object_new_string(devname);
	if (jobj)
		json_object_object_add(jport, name_key, jobj);

	jobj = json_object_new_string(cxl_port_get_host(port));
	if (jobj)
		json_object_object_add(jport, "host", jobj);

	if (!cxl_port_is_enabled(port)) {
		jobj = json_object_new_string("disabled");
		if (jobj)
			json_object_object_add(jport, "state", jobj);
	}

	return jport;
}

struct json_object *util_cxl_port_to_json(struct cxl_port *port,
					  unsigned long flags)
{
	return __util_cxl_port_to_json(port, "port", flags);
}

struct json_object *util_cxl_endpoint_to_json(struct cxl_endpoint *endpoint,
					      unsigned long flags)
{
	return __util_cxl_port_to_json(cxl_endpoint_get_port(endpoint),
				       "endpoint", flags);
}