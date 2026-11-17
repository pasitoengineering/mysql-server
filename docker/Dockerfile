# The official image of the same upstream version, with the worktrees build laid over it,
# so its entrypoint, MYSQL_* env vars and clients work as they do on mysql:9.7.4.
# Build context: this directory, with the release's linux tarballs in dist/.
ARG BASE=mysql:9.7.4-oraclelinux9

# The fork is built with gcc 14; Oracle Linux 9's libstdc++ is gcc 11's, so ship gcc 14's.
FROM amazonlinux:2023 AS libstdcxx

FROM ${BASE} AS fork
ARG TARGETARCH
RUN microdnf install -y tar gzip gcc && microdnf clean all
COPY dist/ /dist/
COPY distance_udf.c /tmp/
COPY --from=libstdcxx /usr/lib64/libstdc++.so.6* /libstdcxx/
RUN set -eux; \
    arch=$([ "$TARGETARCH" = arm64 ] && echo aarch64 || echo x86_64); \
    mkdir /usr/local/mysql; \
    tar -xzf /dist/*-linux-"$arch".tar.gz -C /usr/local/mysql --strip-components=1; \
    gcc -O2 -shared -fPIC -I/usr/local/mysql/include \
      -o /usr/local/mysql/lib/plugin/distance_udf.so /tmp/distance_udf.c; \
    cp -P /libstdcxx/libstdc++.so.6* /usr/local/mysql/lib/private/; \
    cd /usr/local/mysql; \
    rm -rf include man docs lib/*.a bin/mysqltest* bin/mysql_client_test bin/mysql_keyring_encryption_test bin/mysql_test_event_tracking

FROM ${BASE}
LABEL org.opencontainers.image.source=https://github.com/pasitoengineering/mysql-server
LABEL org.opencontainers.image.description="MySQL 9.7.4 with CREATE WORKTREE and a DISTANCE() loadable function"
COPY --from=fork /usr/local/mysql /usr/local/mysql
RUN for f in /usr/local/mysql/bin/*; do ln -sf "$f" /usr/bin/; done \
    && ln -sf /usr/local/mysql/bin/mysqld /usr/sbin/mysqld
COPY fork.cnf /etc/mysql/conf.d/
COPY initdb/ /docker-entrypoint-initdb.d/
