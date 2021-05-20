package com.example.emmc;


import android.content.Context;
import android.content.res.AssetManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class CopyElfs {
    String TAG="Ce_Debug:";
    Context ct;
    String appFileDirectory,executableFilePath;
    AssetManager assetManager;
    List resList;
    final String rootcmd = "sh";
    String cpuType;
    String[] assetsFiles={
            "mmc"
    };

    private Handler handler;

    public CopyElfs(Context c){
        ct=c;
        appFileDirectory = ct.getFilesDir().getPath();
        executableFilePath = appFileDirectory + "/executable";

        // cpuType = Build.SUPPORTED_ABIS[0];
        cpuType = Build.CPU_ABI;
        Log.d(TAG,"CPU_ABI:"+cpuType);
        assetManager = ct.getAssets();
        try {
            resList = Arrays.asList(ct.getAssets().list(cpuType+"/"));
            Log.d(TAG,"get assets list:"+resList.toString());
        } catch (IOException e) {
            Log.e(TAG, "Error list assets folder:", e);
        }
    }
    boolean resFileExist(String filename){
        File f=new File(executableFilePath+"/"+filename);
        if (f.exists())
            return true;
        return false;
    }
    void copyFile(InputStream in, OutputStream out){
        try {
            byte[] buf = new byte[1024];
            int len;
            while ((len = in.read(buf)) > 0) {
                out.write(buf, 0, len);
            }
        } catch (IOException e){
            Log.e(TAG, "Failed to read/write asset file: ", e);
        }
    };
    private void copyAssets(String filename) {
        InputStream in = null;
        OutputStream out = null;
        Log.d(TAG, "Attempting to copy this file: " + filename);

        try {
            in = assetManager.open(cpuType+"/"+filename);
            File outFile = new File(executableFilePath, filename);
            out = new FileOutputStream(outFile);
            copyFile(in, out);
            in.close();
            in = null;
            out.flush();
            out.close();
            out = null;
        } catch(IOException e) {
            Log.e(TAG, "Failed to copy asset file: " + filename, e);
        }
        Log.d(TAG, "Copy success: " + filename);
    }
    public void copyAll2Data(){
        int i;

        File folder=new File(executableFilePath);
        if (!folder.exists()){
            folder.mkdir();
        }

        for(i=0; i<assetsFiles.length; i++){
            if (!resFileExist(assetsFiles[i])){
                copyAssets(assetsFiles[i]);
                File execFile = new File(executableFilePath+"/"+assetsFiles[i]);
                execFile.setExecutable(true);
            }
        }
    }

    public String getExecutableFilePath(){
        return executableFilePath;
    }

    // 向UI线程发送Message
    public void myCallbackFunc(String nMsg, int i) {
        Message tMsg = new Message();
        Bundle tBundle = new Bundle();
        tMsg.what = i;
        if(i == 4)
            tBundle.putString("ERROR", nMsg);
        else
            tBundle.putString("OUT", nMsg);

        tMsg.setData(tBundle);
        handler.sendMessage(tMsg);
    }

    public List<String> callElf(String cmd) throws InterruptedException {
        String tmpText;
        String mmc_elf = getExecutableFilePath() + "/mmc ";
        cmd = mmc_elf + cmd;
        List<String> list = new ArrayList<String>();
        DataOutputStream dos = null;
        BufferedReader br = null;
        BufferedReader err = null;
        String shell = "su";
        try {
            Process p = Runtime.getRuntime().exec(shell);// 获取root

            dos = new DataOutputStream(p.getOutputStream());// 写入流
            br = new BufferedReader(new InputStreamReader(p.getInputStream()));// 输出缓存
            err = new BufferedReader(new InputStreamReader(p.getErrorStream()));// 错误流缓存

            dos.writeBytes(cmd + "\n");Log.i("cmd", "exe cmd: " + cmd);
            dos.flush();
            dos.writeBytes("exit\n");
            dos.flush();

            while ((tmpText = br.readLine()) != null) {
                list.add(tmpText);
            }
            while ((tmpText = err.readLine()) != null) {
                tmpText = "error: " + tmpText + '\n';
                list.add(tmpText);
            }

            // 等待shell子进程执行完成,返回0表示正常结束
            p.waitFor();
        }catch (IOException e){
            e.printStackTrace();
            list.clear();
            String exc = "IOException: " + e.getMessage();
            list.add(exc);
        }
        finally {
            // 关闭流
            if (dos != null) {
                try {
                    dos.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (br != null) {
                try {
                    br.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (err != null) {
                try {
                    err.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return list;
    }

}
