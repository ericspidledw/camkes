/*
 * Copyright 2016, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the term of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(D61_BSD)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <camkes.h>
#include <lib_crc.h>

#define MAX_PKT_SIZE 250

#define PACKET_MAGIC  0x55

#define OPCODE_SET_STATUS              0x01
#define OPCODE_REQUEST_REPORT          0x04
#define OPCODE_CLEAR_FAULT             0x06
#define OPCODE_SET_FLIPPER_EFFORT      0x30
#define OPCODE_SET_FLIPPER_POSITION    0x31
#define OPCODE_REPORT_FLIPPER_POSITION 0x35

#define OPCODE_SET_VEHICLE_VELOCITY    0x21

struct packet {
	uint8_t  magic;
	uint8_t  length;
	uint8_t  crc_low;
	uint8_t  crc_high;
	uint8_t  header_crc;
	uint8_t  payload[MAX_PKT_SIZE];
} __attribute__((packed));

static uint16_t crc16(uint8_t *data, uint16_t size)
{
	uint16_t crc = 0xFFFF;
	for (int i = 0; i < size; i++) {
		crc = UPDATE_CRC_CCITT(crc, data[i]);
	}
	return crc;
}

static void send_packet(uintptr_t udev, void *payload, int len)
{
	struct packet pkt;
	uint16_t crc;

	pkt.magic = PACKET_MAGIC;
	pkt.length = len;
	memcpy(pkt.payload, payload, len);

	crc = crc16(payload, len);
	pkt.crc_low = crc & 0xFF;
	pkt.crc_high = (crc >> 8) & 0xFF;

	pkt.header_crc = (uint8_t)crc16(&pkt.length, 3);

	memcpy(fdata, &pkt, len + 5);
	cdc_write(udev, len + 5);
}

static void receive_packet(uintptr_t udev, void *buf, int len)
{
	int ret;
	char *tmp = calloc(1, MAX_PKT_SIZE);

	ret = cdc_read(udev, MAX_PKT_SIZE);
	memcpy(tmp, fdata, ret);

	memcpy(buf, tmp, ret < len ? ret : len);

	free(tmp);
}

void set_flipper_effort(uintptr_t udev, uint16_t effort)
{
	char *payload = calloc(1, 3);

	payload[0] = OPCODE_SET_FLIPPER_EFFORT;
	payload[1] = effort & 0xFF;
	payload[2] = (effort & 0xFF00) >> 8;

	send_packet(udev, payload, 3);

	free(payload);
}

void clear_fault(uintptr_t udev, uint16_t fault)
{
	char *payload = calloc(1, 3);

	payload[0] = OPCODE_CLEAR_FAULT;
	payload[1] = fault & 0xFF;
	payload[2] = (fault & 0xFF00) >> 8;

	send_packet(udev, payload, 3);

	free(payload);
}

void set_status(uintptr_t udev, uint8_t status)
{
	char *payload = calloc(1, 2);

	payload[0] = OPCODE_CLEAR_FAULT;
	payload[1] = status;

	send_packet(udev, payload, 2);

	free(payload);
}

void set_flipper_postion(uintptr_t udev, int angle, int velocity)
{
	char *payload = calloc(1, 5);

	payload[0] = OPCODE_SET_FLIPPER_POSITION;
	payload[1] = angle & 0xFF;
	payload[2] = (angle & 0xFF00) >> 8;
	payload[3] = velocity & 0xFF;
	payload[4] = (velocity & 0xFF00) >> 8;

	send_packet(udev, payload, 5);

	free(payload);
}

uint16_t report_flipper_postion(uintptr_t udev)
{
	uint16_t angle;
	char *payload = calloc(1, 2);

	payload[0] = OPCODE_REQUEST_REPORT;
	payload[1] = OPCODE_REPORT_FLIPPER_POSITION;

	send_packet(udev, payload, 2);

	receive_packet(udev, payload, 2);

	angle = (payload[1] << 8) | payload[0];

	free(payload);

	return angle;
}

void set_vehicle_velocity(uintptr_t udev, uint16_t drive, uint16_t turn)
{
	char *payload = calloc(1, 5);

	payload[0] = OPCODE_SET_VEHICLE_VELOCITY;
	payload[1] = drive & 0xFF;
	payload[2] = (drive & 0xFF00) >> 8;
	payload[3] = turn & 0xFF;
	payload[4] = (turn & 0xFF00) >> 8;

	send_packet(udev, payload, 5);

	free(payload);
}

