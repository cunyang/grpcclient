package com.example.clientdemo;

import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RestController;

import java.util.logging.Logger;
import example.*;

@RestController
public class DataAlignController {

    private static final Logger logger = Logger.getLogger(DataAlignController.class.getName());

    @GetMapping("/{address}")
    public String sayHello(@PathVariable String address) {
        byte[] csvFile = getFileWithGrpc(address, "/home/cunyang/Documents/generate/test.csv");
        return "success";
    }

    private byte[] getFileWithGrpc(String address, String path) {
        ManagedChannel channel = ManagedChannelBuilder.forAddress(address, 50051).usePlaintext().build();
        DataAlignGrpc.DataAlignBlockingStub stub = DataAlignGrpc.newBlockingStub(channel);
        DataAlignOuterClass.StringValue filePath = DataAlignOuterClass.StringValue.newBuilder().setValue(path).build();
        DataAlignOuterClass.CsvFile csvFile = stub.getData(filePath);
        byte[] bytes = csvFile.getData().toByteArray();
        channel.shutdown();
        return bytes;
    }

    private byte[] getFileLocally(){
        return null;
    }

}
