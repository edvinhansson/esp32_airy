idf_component_register(SRCS "dht.c" "main.c" "network.c" "wifi.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES "certs/rootca.crt")