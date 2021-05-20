package com.example.emmc;


import androidx.appcompat.app.AppCompatActivity;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.Spinner;
import com.leon.lfilepickerlibrary.LFilePicker;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity {
    //public static native String emmc();

    static private List<String> buffer = new ArrayList<String>();
    private String mmcblkpath = new String();
    private String filepath = new String();
    private String IOExceptions = new String();
    private TextView bin;
    private CopyElfs ce;
    private TextView tv_fw; // 版本号显示文本框
    private Button chose_bin_button;//选择bin文件的按钮//
    private Button help;
    private Button read;
    private Button rwtest;
    private TextView JNITest;
    static boolean IsSuccess = false;

    private int REQUESTCODE_FROM_ACTIVITY = 1000;
    private List<String> list_update=null;
    private my_SpinnerAdapter adapter_update;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // 初始化
        ce = new CopyElfs(getBaseContext());
        ce.copyAll2Data();
        tv_fw = (TextView)findViewById(R.id.fw_version);
        help=(Button)findViewById(R.id.help);//帮助按钮，按下跳转//
        bin=(TextView)findViewById(R.id.bin_file);
        rwtest=(Button)findViewById(R.id.rwtest);
        JNITest=(TextView)findViewById(R.id.JNITest);
        help.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                //Intent intent=new Intent(MainActivity.this,HelpActivity.class);
                //startActivity(intent);
                JNITest.setText(""+JniUtil.stringFromJNI());
            }
        });
        read=(Button)findViewById(R.id.read);//查看信息按钮，下面是触发事件
        read.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                Intent intent=new Intent(MainActivity.this,Read_ExctActivity.class);
                //intent.putExtra("IC_path",IC_path);//这个是选择芯片的地址，查看信息需要他//
                startActivity(intent);
            }
        });
        chose_bin_button=(Button)findViewById(R.id.chose_bin);
        chose_bin_button.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                new LFilePicker()
                        .withActivity(MainActivity.this)
                        .withRequestCode(REQUESTCODE_FROM_ACTIVITY)
                        .withStartPath("/sdcard/")//指定初始显示路径
                        .withIsGreater(false)//过滤文件大小 小于指定大小的文件
                        .withFileSize(500 * 1024)//指定文件大小为500K
                        .withFileFilter(new String[]{".txt", ".png", ".docx"})
                        .withMutilyMode(false)
                        .start();
            }
        });

    }

    // 按键事件，根据id触发
    public void myclick(View v) throws IOException {
        switch (v.getId()){
            case R.id.chose_emmc:
                //EmmcOnclick();
                ProcessTest();
                break;
            case  R.id.update:
                UpdateOnclick();
                break;
            case R.id.rwtest:{
                Intent intent=new Intent(this,RWTest.class);
                ShowMessage("mmcblkpath is : "+mmcblkpath);
                intent.putExtra("path",mmcblkpath);
                startActivity(intent);
            }

        }
       // startActivity(intent);
    }
    /*
    * 点击 选择emmc 事件：
    * 首先执行mmc.elf的return命令用来输出/dev/block下的mmcblk，将其存储到List<String>类型的buffer中
    * 然后将List<String>类型的buffer转换为String[]，用此String[]来作为对话框列表的初值
    * 选择对话框列表中的mmcblk后，将此值赋值给private String emmc
    * */
    private void EmmcOnclick() {
        // 获取 mmcblk 命令，初始化可读写的devices
        try {
            buffer = ce.callElf("return");
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        // 显示对话框
        showEmmcDialog();
    }
    private void ProcessTest() throws IOException {
        String path =getApplicationContext().getFilesDir().getAbsolutePath();
        Log.e("路径",path);
        Process process=Runtime.getRuntime().exec("/data/user/0/com.examp le.emmc/files/mmc",null,null);
        BufferedInputStream inputStream = new BufferedInputStream(
                process.getInputStream());
        BufferedReader bufferedReader = new BufferedReader(
                new InputStreamReader(inputStream));
        String line;
        while (bufferedReader.read() != -1) {
            line = bufferedReader.readLine();
            Log.e("输出的信息",line);
        }
        bufferedReader.close();
        inputStream.close();
        process.destroy();
    }

    /*
    * 选择emmc对话框显示
    * */
    private void showEmmcDialog() {
        String[] devices = list_to_array();

        // 会话框设置
        AlertDialog.Builder dialog = new AlertDialog.Builder (this);
        dialog.setTitle ("选择emmc块");

        // 有无获取到设备
        if(devices.length == 0){
            dialog.setTitle("未找到eMMC");
            if(IOExceptions.isEmpty())
                dialog.setMessage("该设备无SCY可读的mmcblk");
            else
                dialog.setMessage("错误原因:\n" + IOExceptions);
        }
        else{
            dialog.setTitle ("选择eMMC块");
            dialog.setItems(devices, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    mmcblkpath = devices[which].toString();
                    ShowMessage(mmcblkpath);
                    ShowEmmcblkFw(mmcblkpath);
                }
            });
        }
        // show()方法显示对话框
        dialog.show ();
    }

    /*
     * 点击 更新 事件：
     * 首先判断emmc和filepath路径是否为空，即是否有选择emmc和文件
     * 然后执行mmc.elf的ffu将输出流存储到List<String>类型的buffer中，
     * 将buffer转换为String，并检测输出中是否有Success来判断是否更新成功
     * */
    private void UpdateOnclick() {
        if(filepath.isEmpty() || mmcblkpath.isEmpty()) {
            ShowMessage("文件或emmc芯片未选择");
            return;
        }

        IsSuccess = false;    // 状态初始化
        try {
            buffer = ce.callElf("ffu " + filepath + " " + mmcblkpath);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        showUpdateDialog();
    }

    /*
    * 更新结果显示对话框
    * */
    private void showUpdateDialog() {
        AlertDialog.Builder dialog = new AlertDialog.Builder (this);
        dialog.setIcon(R.drawable.update);
        dialog.setTitle("更新固件");
        String updateOutString = DealUpdateString();
        if(IsSuccess) {
            dialog.setMessage ("更新成功！！！");
            ShowEmmcblkFw(mmcblkpath);  // 刷新当前emmc的版本显示文本框
        }
        else {
            dialog.setMessage("更新失败！！！\n" + updateOutString);
        }
        // 点击确定
        dialog.setPositiveButton ("确定", new DialogInterface.OnClickListener () {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                // tv_fw.setText(null);
                // MainActivity.this.finish();
            }
        });
        dialog.show ();
    }

    /*
    * 在TextView中显示选择的mmcblk的版本号
    * 通过extcsd read mmcblk的输出流的fw version输出来截取版本号
    * */
    private void ShowEmmcblkFw(String mmcblk) {
        // 获取 mmcblk 版本号: fw version
        String fw = new String();
        try {
            buffer = ce.callElf("extcsd read " + mmcblkpath);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        List<String> err =  new ArrayList<String>();
        for (int i = 0; i < buffer.size(); i++) {
            if(buffer.get(i).contains("fw version"))
                fw =  String.valueOf(buffer.get(i));
            else if(buffer.get(i).startsWith("error:")){
                err.add(String.valueOf(buffer.get(i)));
            }
        }
        buffer.clear();
        if(!fw.isEmpty()){
            fw = mmcblkpath + " " + fw;
            tv_fw.setText(fw);
        }else{
            tv_fw.setText(null);
            tv_fw.append("读取"+mmcblkpath+"版本号失败\n");
            for (int i = 0; i < err.size(); i++)
                tv_fw.append(err.get(i).toString());
        }
    }

    private void ShowMessage(String str) {
        Toast.makeText(MainActivity.this, str, Toast.LENGTH_LONG).show();
    }

    /*
    * 将list类型转换为String[]
    * */
    private String[] list_to_array() {
        IOExceptions = "";
        List<String> list = new ArrayList<String>();
        for (int i = 0; i < buffer.size(); i++) {
            if(buffer.get(i).startsWith("/dev/block"))
                list.add(buffer.get(i));
            else if(buffer.get(i).startsWith("IOException:"))
                IOExceptions += buffer.get(i) + '\n';
        }
        String[] array = new String[list.size()];
        for (int i = 0; i < list.size(); i++) {
                array[i] = String.valueOf(list.get(i));
        }
        buffer.clear();
        return array;
    }


    /*
    * 处理更新事件的输出流，并判断是否更新成功
    * 将mmc.elf执行ffu功能的输出流（已存入到buffer中），由list<String>转换到String类型
    * 并通过字符串string是否包含Success来判断是否更新成功
    * */
    private String DealUpdateString() {
        String str = new String();
        for (int i = 0; i < buffer.size(); i++) {
            str = String.valueOf(buffer.get(i)) + "\n";
            if(str.contains("Success"))
                IsSuccess = true;
        }
        buffer.clear();
        return str;
    }


    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode == RESULT_OK) {
            if (requestCode == REQUESTCODE_FROM_ACTIVITY) {
                Bundle b=data.getExtras();
                Object[] lstName=b.keySet().toArray();

                for(int i=0;i<lstName.length;i++)

                {

                    String keyName=lstName[i].toString();

                    Log.e(keyName,String.valueOf(b.get(keyName)));

                }
                //如果是文件选择模式，需要获取选择的所有文件的路径集合
                //List<String> list = data.getStringArrayListExtra(Constant.RESULT_INFO);//Constant.RESULT_INFO == "paths"
                list_update = data.getStringArrayListExtra("paths");
                Toast.makeText(getApplicationContext(), "选中了" + list_update.size() + "个文件", Toast.LENGTH_SHORT).show();
                bin.setText(list_update.get(0));
            }
        }

    }
}