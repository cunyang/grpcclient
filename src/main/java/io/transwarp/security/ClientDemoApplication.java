package io.transwarp.security;

import io.transwarp.security.graphene.GrapheneJNI;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

@SpringBootApplication
public class ClientDemoApplication {

    public static GrapheneJNI graphene;

    static {
        System.loadLibrary("GrapheneJNI");
    }

    public static void main(String[] args) {
        graphene = new GrapheneJNI();
        SpringApplication.run(ClientDemoApplication.class, args);
    }

}
