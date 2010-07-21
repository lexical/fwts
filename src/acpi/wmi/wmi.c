/*
 * Copyright (C) 2010 Canonical
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "fwts.h"

typedef enum {
	FWTS_WMI_EXPENSIVE 	= 0x00000001,
	FWTS_WMI_METHOD		= 0x00000002,
	FWTS_WMI_STRING		= 0x00000004,
	FWTS_WMI_EVENT		= 0x00000008
} fwts_wmi_flags;

/*
 *  Packed WMI WDG data 
 */
typedef struct {
	uint8	guid[16];			/* GUID */
	union {
		uint8 	obj_id[2];		/* Object Identifier */
		struct {
			uint8	notify_id;	/* Notify Identifier */
			uint8	reserved;	/* Reserved? */
		};
	};
	uint8	instance;			/* Instance */
	uint8	flags;				/* fwts_wmi_flags */
} __attribute__ ((packed)) fwts_guid_info;

static char *wmi_headline(void)
{
	return "Extract and analyse Windows Management Instrumentation (WMI).";
}

static int wmi_init(fwts_framework *fw)
{
	if (fw->acpi_table_path == NULL)
		if (fwts_check_root_euid(fw))
			return FWTS_ERROR;

	if (fwts_check_executable(fw, fw->iasl, "iasl"))
		return FWTS_ERROR;

	return FWTS_OK;
}

#define CONSUME_WHITESPACE(str)		\
	while (*str && isspace(*str))	\
		str++;			\
	if (*str == '\0') return;	\

char *wmi_wdg_flags_to_text(const fwts_wmi_flags flags)
{
	static char buffer[256];

	*buffer = 0;

	if (flags & FWTS_WMI_EXPENSIVE)
		strcat(buffer, "WMI_EXPENSIVE ");
	if (flags & FWTS_WMI_METHOD)
		strcat(buffer, "WMI_METHOD");
	if (flags & FWTS_WMI_STRING)
		strcat(buffer, "WMI_STRING");
	if (flags & FWTS_WMI_EVENT)
		strcat(buffer, "WMI_EVENT ");
	
	return buffer;
}

static void wmi_parse_wdg_data(fwts_framework *fw, int size, uint8 *wdg_data)
{
	int i;
	int advice_given = 0;

	fwts_guid_info *info = (fwts_guid_info *)wdg_data;

	for (i=0; i<(size / sizeof(fwts_guid_info)); i++) {
		uint8 *guid = info->guid;
		char guidstr[37];

		snprintf(guidstr, sizeof(guidstr),
			"%02X%02X%02X%02X-%02X%02X-%02X%02X-%02x%02X-%02X%02X%02X%02X%02X%02X", 
			guid[3], guid[2], guid[1], guid[0],
			guid[5], guid[4],
			guid[7], guid[6],
			guid[8], guid[9],
			guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]);

		if (info->flags & FWTS_WMI_METHOD) {
			fwts_log_info(fw, "Found WMI method WM%c%c with GUID %s\n", info->obj_id[0], info->obj_id[1], guidstr);
		} else if (info->flags & FWTS_WMI_EVENT) {
			fwts_log_info(fw, "Found WMI event with GUID: %s\n", guidstr);
			if (!advice_given) {
				advice_given = 1;
				fwts_log_nl(fw);
				fwts_log_advice(fw, 	
					"ADVICE: A WMI driver probably needs to be written for this event.");
				fwts_log_advice(fw,
					"It can checked for using: wmi_has_guid(\"%s\").\n", guidstr);
				fwts_log_advice(fw, 
					"One can install a notify handler using wmi_install_notify_handler(\"%s\", handler, NULL).  ", guidstr);
				fwts_log_advice(fw, 
					"http://lwn.net/Articles/391230 describes how to write an appropriate driver.");
				fwts_log_nl(fw);
			}
		} else {
			char *flags = wmi_wdg_flags_to_text(info->flags);
			fwts_log_info(fw, "Found WMI object ID %c%c, notifier ID: %02X, GUID %s\n", 
				info->obj_id[0], info->obj_id[1], info->notify_id, guidstr, flags);
		}
		info++;
	}
}

static void wmi_get_wdg_data(fwts_framework *fw, fwts_list_link *item, int size, uint8 *wdg_data)
{
	char *str;
	uint8 *data = wdg_data;

	for (;item != NULL; item=item->next) {
		int i;
		str = fwts_text_list_text(item);
		CONSUME_WHITESPACE(str);

		if (*str == '}') break;

		if (strncmp(str, "/*", 2) == 0) {
			str = strstr(str, "*/");
			if (str)
				str += 2;
			else
				continue;
		}
		CONSUME_WHITESPACE(str);
		
		for (i=0;i<8;i++) {
			if (strncmp(str, "0x", 2))
				break;
			*data = strtoul(str, NULL, 16);
			str+=4;
			if (*str != ',') break;
			str++;
			if (!isspace(*str)) break;
			str++;
			data++;
			if (data > wdg_data + size) {
				fwts_failed_high(fw, "_WDG buffer was more than %d bytes long!", size);
				return;
			}
		}
	}
	return;
}

static void wmi_parse_for_wdg(fwts_framework *fw, fwts_list_link *item)
{
	uint8 *wdg_data;
	int size;
	char *str = fwts_text_list_text(item);

	/* Parse Name(_WDG, Buffer, (0xXX) */

	CONSUME_WHITESPACE(str);

	if (strncmp(str, "Name", 4))
		return;
	str += 4;

	CONSUME_WHITESPACE(str);

	if (*str != '(') return;
	str++;

	CONSUME_WHITESPACE(str);

	if (strncmp(str, "_WDG",4))
		return;
	str+=4;

	CONSUME_WHITESPACE(str);

	if (*str != ',') return;
	str++;

	CONSUME_WHITESPACE(str);

	if (strncmp(str, "Buffer",6))
		return;
	str+=6;

	CONSUME_WHITESPACE(str);

	if (*str != '(') return;
	str++;

	CONSUME_WHITESPACE(str);

	size = strtoul(str, NULL, 16);
		
	item = item->next;
	if (item == NULL) return;

	str = fwts_text_list_text(item);
	CONSUME_WHITESPACE(str);
	if (*str != '{') return;

	item = item->next;
	if (item == NULL) return;

	if ((wdg_data = calloc(1, size)) != NULL) {
		wmi_get_wdg_data(fw, item, size, wdg_data);
		wmi_parse_wdg_data(fw, size, wdg_data);
		free(wdg_data);
	}
}

static int wmi_table(fwts_framework *fw, char *table, int which)
{
	fwts_list_link *item;
	fwts_list* iasl_output;

	iasl_output = fwts_iasl_disassemble(fw, table, which);
	if (iasl_output == NULL) {
		return FWTS_ERROR;
	}

	for (item = iasl_output->head; item != NULL; item = item->next) {
		wmi_parse_for_wdg(fw, item);
	}
	fwts_text_list_free(iasl_output);

	return FWTS_OK;
}

static int wmi_DSDT(fwts_framework *fw)
{
	return wmi_table(fw, "DSDT", 0);
}

static int wmi_SSDT(fwts_framework *fw)
{
	int i;

	for (i=0; i < 100; i++) {
		int ret = wmi_table(fw, "SSDT", i);
		if (ret == 2)
			return FWTS_OK;	/* Hit the last table */
		if (ret != FWTS_OK)
			return FWTS_ERROR;
	}

	return FWTS_OK;
}

static fwts_framework_tests wmi_tests[] = {
	wmi_DSDT,
	wmi_SSDT,
	NULL
};

static fwts_framework_ops wmi_ops = {
	wmi_headline,
	wmi_init,	
	NULL,
	wmi_tests
};

FWTS_REGISTER(wmi, &wmi_ops, FWTS_TEST_ANYTIME, FWTS_BATCH);
