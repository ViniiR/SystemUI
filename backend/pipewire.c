#include <pipewire-0.3/pipewire/pipewire.h>

struct app_data {
    struct pw_main_loop *loop;
    struct pw_context *context;
    struct pw_core *core;
    struct pw_registry *registry;
    struct spa_hook registry_listener;
};

static void registry_event_global(void *data, uint32_t id,
                                  uint32_t permissions, const char *type,
                                  uint32_t version, const struct spa_dict *props) {
    if (!props) return;

    // We check for PipeWire Nodes (streams and devices are nodes)
    if (strcmp(type, PW_TYPE_INTERFACE_Node) == 0) {
        const char *media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
        const char *node_name = spa_dict_lookup(props, PW_KEY_NODE_NAME);
        const char *node_desc = spa_dict_lookup(props, PW_KEY_NODE_DESCRIPTION);
        const char *app_name = spa_dict_lookup(props, PW_KEY_APP_NAME);

        // Filter specifically for audio streams/sinks/sources
        if (media_class && strstr(media_class, "Audio") != NULL) {
            printf("Found Audio Node [ID: %u]\n", id);
            printf("  Class:       %s\n", media_class);
            printf("  Name:        %s\n", node_name ? node_name : "N/A");
            printf("  Description: %s\n", node_desc ? node_desc : "N/A");
            if (app_name) {
                printf("  Application: %s\n", app_name);
            }
            printf("----------------------------------------\n");
        }
    }
}

static const struct pw_registry_events registry_events = {
    PW_VERSION_REGISTRY_EVENTS,
    .global = registry_event_global,
};

int init_pipewire() {
    struct app_data app = {0};

    pw_init(0, NULL);

    app.loop = pw_main_loop_new(NULL);
    app.context = pw_context_new(pw_main_loop_get_loop(app.loop), NULL, 0);
    app.core = pw_context_connect(app.context, NULL, 0);

    if (!app.core) {
        fprintf(stderr, "Failed to connect to PipeWire core\n");
        return -1;
    }

    // Get the registry and add the event listener
    app.registry = pw_core_get_registry(app.core, PW_VERSION_REGISTRY, 0);
    pw_registry_add_listener(app.registry, &app.registry_listener, &registry_events, &app);

    // Run the loop to process global object announcements
    pw_main_loop_run(app.loop);

    // Cleanup
    pw_proxy_destroy((struct pw_proxy *)app.registry);
    pw_core_disconnect(app.core);
    pw_context_destroy(app.context);
    pw_main_loop_destroy(app.loop);
    pw_deinit();

    return 0;
}
