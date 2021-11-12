FROM 172.16.1.99/aip/ubuntu-base:master

#Fix bug of "error while loading shared libraries: libjli.so"
RUN sed -i -e '$a/usr/java/jdk1.8.0_211/lib/amd64/jli' /etc/ld.so.conf
RUN ldconfig

ENV APP_HOME /root

COPY target/clientDemo-0.0.1-SNAPSHOT.jar ${APP_HOME}/app.jar
COPY csv/*.csv ${APP_HOME}/

WORKDIR ${APP_HOME}

ENTRYPOINT ["java", "-jar", "-Xdebug", "-Xrunjdwp:transport=dt_socket,suspend=n,server=y,address=5005", "-Dcom.sun.management.jmxremote.port=1616", "-Dcom.sun.management.jmxremote.rmi.port=1616", "-Dcom.sun.management.jmxremote.ssl=false", "-Dcom.sun.management.jmxremote.authenticate=false", "-Dcom.sun.management.jmxremote.local.only=false", "-Djava.rmi.server.hostname=172.26.120.15", "-Xmx2G", "-XX:MetaspaceSize=1G", "-XX:MaxMetaspaceSize=1G", "-XX:+UseSerialGC", "app.jar"]