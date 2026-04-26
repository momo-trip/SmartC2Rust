#include <url.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Check if components exist in the URL
void check_component_exists(const url_data_t *data, const char *component_name) {
    int exists = 0;
    
    if (strcmp(component_name, "href") == 0) exists = data->href != NULL;
    else if (strcmp(component_name, "auth") == 0) exists = data->auth != NULL;
    else if (strcmp(component_name, "protocol") == 0) exists = data->protocol != NULL;
    else if (strcmp(component_name, "port") == 0) exists = data->port != NULL;
    else if (strcmp(component_name, "hostname") == 0) exists = data->hostname != NULL;
    else if (strcmp(component_name, "host") == 0) exists = data->host != NULL;
    else if (strcmp(component_name, "pathname") == 0) exists = data->pathname != NULL;
    else if (strcmp(component_name, "path") == 0) exists = data->path != NULL;
    else if (strcmp(component_name, "hash") == 0) exists = data->hash != NULL;
    else if (strcmp(component_name, "search") == 0) exists = data->search != NULL;
    else if (strcmp(component_name, "query") == 0) exists = data->query != NULL;
    
    printf("EXISTS %s: %d\n", component_name, exists);
}

// Validate protocol
int check_protocol(const char *protocol) {
    int is_valid = url_is_protocol(protocol);
    printf("DEBUG: Protocol %s validation result: %d\n", protocol, is_valid);
    printf("PROTOCOL_VALID %s: %d\n", protocol, is_valid);
    return is_valid;
}

// Get and output component value
void print_component(const char *url, const char *component_name,
                    const char* (*getter_func)(const char*)) {
    const char *value = getter_func(url);
    if (value) {
        printf("VALUE %s: %s\n", component_name, value);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <url_or_command>\n", argv[0]);
        return 1;
    }

    char *input = argv[1];
    
    // Process protocol validation command
    if (strncmp(input, "CHECK_PROTOCOL:", 15) == 0) {
        char *protocol = input + 15;  // Get the part after "CHECK_PROTOCOL:"
        int result = check_protocol(protocol);
        return !result;
    }

    // Process URL parsing
    url_data_t *parsed = url_parse(input);
    
    if (!parsed) {
        fprintf(stderr, "Error: Failed to parse URL\n");
        return 1;
    }

    // Check component existence
    check_component_exists(parsed, "href");
    check_component_exists(parsed, "auth");
    check_component_exists(parsed, "protocol");
    check_component_exists(parsed, "port");
    check_component_exists(parsed, "hostname");
    check_component_exists(parsed, "host");
    check_component_exists(parsed, "pathname");
    check_component_exists(parsed, "path");
    check_component_exists(parsed, "hash");
    check_component_exists(parsed, "search");
    check_component_exists(parsed, "query");

    // Output component values
    print_component(input, "protocol", (const char* (*)(const char*))url_get_protocol);
    print_component(input, "auth", (const char* (*)(const char*))url_get_auth);
    print_component(input, "hostname", (const char* (*)(const char*))url_get_hostname);
    print_component(input, "host", (const char* (*)(const char*))url_get_host);
    print_component(input, "pathname", (const char* (*)(const char*))url_get_pathname);
    print_component(input, "path", (const char* (*)(const char*))url_get_path);
    print_component(input, "search", (const char* (*)(const char*))url_get_search);
    print_component(input, "query", (const char* (*)(const char*))url_get_query);
    print_component(input, "hash", (const char* (*)(const char*))url_get_hash);
    print_component(input, "port", (const char* (*)(const char*))url_get_port);

    url_free(parsed);
    return 0;
}