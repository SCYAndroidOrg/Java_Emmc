package com.example.emmc;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

class test {

    public static void main(String[] args) throws IOException {
        // TODO Auto-generated method stub
        File file = new File("E:\\edge2.txt");
        FileInputStream in = new FileInputStream(file);
        BufferedInputStream bufferedIn = new BufferedInputStream(in);

        int[] len = {0, 0};
        byte[] buffer = new byte[1024];
        long point1 = 0;
        long point2 = 0;
        long point3 = 0;
        double readTime = 0;
        double writeTime = 0;
        point1 = System.currentTimeMillis();
        while ((len[0] = bufferedIn.read(buffer, 0, 1024)) != -1) {
            //point2 = System.currentTimeMillis();
            //readTime += (point2 - point1) * 1.0 / 1000;
            len[1] += len[0];
            //point1 = point2;
        }
        point2 = System.currentTimeMillis();
        readTime += (point2 - point1) * 1.0 / 1000;
        System.out.println(len[1] + " " + readTime);
    }

}
