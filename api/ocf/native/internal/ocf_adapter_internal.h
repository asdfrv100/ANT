#ifndef __OCF_ADAPTER_INTERNAL_H__
#define __OCF_ADAPTER_INTERNAL_H__

#include "ll.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

void ocf_adapter_init_internal(void);
void ocf_adapter_deinit_internal(void);

void ocf_adapter_start_internal(void);
void ocf_adapter_stop_internal(void);

bool ocf_adapter_setPlatform_internal(const char *mfg_name);
bool ocf_adapter_addDevice_internal(const char *uri, const char *resource_type,
                                    const char *name, const char *spec_version,
                                    const char *data_model_version);
bool ocf_adapter_addResource_internal(void *ocf_resource_nobject);

void ocf_adapter_repStartRootObject_internal(void);
void ocf_adapter_repSetBoolean_internal(const char *key, bool value);
void ocf_adapter_repSetInt_internal(const char *key, int value);
void ocf_adapter_repSetDouble_internal(const char *key, double value);
void ocf_adapter_repSetString_internal(const char *key, const char *value);
void ocf_adapter_repEndRootObject_internal(void);
void ocf_adapter_sendResponse_internal(void *ocf_request_nobject,
                                       int status_code);

struct oa_discovery_event_data_s {
  char *uri;
  ll_t *types;
  int interface_mask;
  void *endpoint;

  pthread_mutex_t sync_mutex;
  pthread_cond_t sync_cond;
};
typedef struct oa_discovery_event_data_s oa_discovery_event_data_t;
void oa_discovery_event_data_destroyer(void *item) {
  oa_discovery_event_data_t *event;
  event = (oa_discovery_event_data_t *)item;
  free(event->uri);
  ll_delete(event->types);
  free(event);
}
void oa_discovery_event_types_destroyer(void *item) { free(item); }
void ocf_endpoint_destroy(void *handle);

bool ocf_adapter_isDiscovering_internal(void);
void ocf_adapter_stopDiscovery_internal(void);
bool ocf_adapter_discovery_internal(const char *resource_type);

struct oa_client_response_event_data_s {
  void *endpoint;
  char *payload_string;
  size_t payload_string_length;
  int status_code;
  int request_id;
};
typedef struct oa_client_response_event_data_s oa_client_response_event_data_t;
void oa_response_event_data_destroyer(void *item) {
  oa_client_response_event_data_t *event;
  event = (oa_client_response_event_data_t *)item;
  free(event->payload_string);
  free(event);
}

#define DECLARE_OCF_REQUEST_INTERNAL(type)                                     \
  bool type##_internal(int requestId, void *ocf_endpoint_nobject,              \
                       const char *uri, const char *query, int qos)

DECLARE_OCF_REQUEST_INTERNAL(ocf_adapter_observe);
bool ocf_adapter_stopObserve_internal(void *ocf_endpoint_nobject,
                                      const char *uri);
DECLARE_OCF_REQUEST_INTERNAL(ocf_adapter_get);
DECLARE_OCF_REQUEST_INTERNAL(ocf_adapter_delete);
DECLARE_OCF_REQUEST_INTERNAL(ocf_adapter_initPost);
DECLARE_OCF_REQUEST_INTERNAL(ocf_adapter_initPut);
bool ocf_adapter_post_internal(void);
bool ocf_adapter_put_internal(void);

void initOCFAdapter(void);

#define OCF_REQUEST_INTERNAL_HANDLER(type)                                     \
  static void type##_handler(oc_client_response_t *data) {                     \
    oa_client_response_event_data_t *event_data;                               \
    event_data = (oa_client_response_event_data_t *)malloc(                    \
        sizeof(oa_client_response_event_data_t));                              \
                                                                               \
    oc_endpoint_copy((oc_endpoint_t *)event_data->endpoint, data->endpoint);   \
                                                                               \
    event_data->payload_string_length =                                        \
        oc_rep_to_json(data->payload, NULL, 0, true);                          \
    event_data->payload_string =                                               \
        (char *)malloc(event_data->payload_string_length + 1);                 \
    oc_rep_to_json(data->payload, event_data->payload_string,                  \
                   event_data->payload_string_length + 1, true);               \
                                                                               \
    event_data->status_code = data->code;                                      \
                                                                               \
    event_data->request_id = (int)data->user_data;                             \
                                                                               \
    EMIT_ANT_ASYNC_EVENT(type, event_data->request_id, (void *)event_data);    \
    return;                                                                    \
  }
#define OCF_REQUEST_INTERNAL(type, ocf_request_function)                       \
  bool type##_internal(int requestId, void *ocf_endpoint_nobject,              \
                       const char *uri, const char *query, int qos) {          \
    oc_endpoint_t *endpoint = (oc_endpoint_t *)ocf_endpoint_nobject;           \
    oc_qos_t oc_qos = (qos == HIGH_QOS) ? HIGH_QOS : LOW_QOS;                  \
    return ocf_request_function(uri, endpoint, query, &type##_handler, oc_qos, \
                                (void *)requestId);                            \
  }

#endif /* !defined(__OCF_ADAPTER_INTERNAL_H__) */