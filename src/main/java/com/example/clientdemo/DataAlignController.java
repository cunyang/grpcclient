package com.example.clientdemo;

import example.DataAlignGrpc;
import example.DataAlignOuterClass;
import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import org.apache.commons.csv.CSVFormat;
import org.apache.commons.csv.CSVRecord;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

@RestController
public class DataAlignController {

    @PostMapping("/")
    public String sayHello(@RequestParam("local-file-path") String localFilePath,
                           @RequestParam("grpc-file-path") String grpcFilePath,
                           @RequestParam("address") String address) throws IOException {
        long startTime = System.currentTimeMillis();
        int count = 0;
        FileWriter csvWriter = new FileWriter("/tmp/result.csv");
        Iterable<CSVRecord> localRecords = CSVFormat.DEFAULT.parse(new FileReader(localFilePath));
        List<String> grpcRecords = getListFromGrpc(address, grpcFilePath);
        for (CSVRecord record : localRecords) {
            if (grpcRecords.contains(record.get(0))) {
                count++;
                csvWriter.append(record.get(0)).append("\n");
            }
        }
        csvWriter.flush();
        csvWriter.close();
        long endTime = System.currentTimeMillis();
        return "样本对齐程序运行时间：" + (endTime - startTime) + "毫秒, 交集数量:" + count;
    }

    private List<String> getListFromGrpc(String address, String path) throws IOException {
        List<String> result = new ArrayList<>();
        ManagedChannel channel = ManagedChannelBuilder.forAddress(address, 50051).usePlaintext().build();
        DataAlignGrpc.DataAlignBlockingStub stub = DataAlignGrpc.newBlockingStub(channel);
        DataAlignOuterClass.StringValue filePath = DataAlignOuterClass.StringValue.newBuilder().setValue(path).build();
        DataAlignOuterClass.CsvFile csvFile = stub.getData(filePath);
        byte[] bytes = csvFile.getData().toByteArray();
        channel.shutdown();
        ByteArrayInputStream stream = new ByteArrayInputStream(bytes);
        CSVFormat.DEFAULT.parse(new InputStreamReader(stream, StandardCharsets.UTF_8)).forEach(row -> result.add(row.get(0)));
        return result;
    }
}
