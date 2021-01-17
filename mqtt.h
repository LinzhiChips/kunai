/*
 * mqtt.h - MQTT setup and input processing
 *
 * Copyright (C) 2021 Linzhi Ltd.
 *
 * This work is licensed under the terms of the MIT License.
 * A copy of the license can be found in the file COPYING.txt
 */

#ifndef KUNAI_MQTT_H
#define	KUNAI_MQTT_H

#define	MQTT_TOPIC_TIME		"/daemon/%s/time"
#define	MQTT_TOPIC_LOG		"/daemon/%s/log"
#define	MQTT_TOPIC_CURRENT	"/daemon/%s/current"
#define	MQTT_TOPIC_CURRENT_GET	"/daemon/%s/current-get"
#define	MQTT_TOPIC_START	"/daemon/%s/start"
#define	MQTT_TOPIC_STOP		"/daemon/%s/stop"
#define	MQTT_TOPIC_CYCLE	"/daemon/%s/cycle"


void mqtt_loop(void);
void mqtt_setup(void);

#endif /* !KUNAI_MQTT_H */
