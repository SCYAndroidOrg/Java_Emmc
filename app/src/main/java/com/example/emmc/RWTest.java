package com.example.emmc;

import androidx.appcompat.app.AppCompatActivity;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;

public class RWTest extends AppCompatActivity {
    /*读写测试的act*/
    //测试模式
    private String[] TestModelArray ={"random","serial"};

    private TextView FrequencyText;
    //测试次数
    private String[] TestFrequencyArray= {"1","2","3"};
    private String TestModel="";
    private String TestFrequency="";
    private double WriteSpeed=0.0;
    private double ReadSpeed=0.0;
    private double CorrectRate=0.0;
    private int Offset=0;
    private int Length=0;
    static private List<String> buffer = new ArrayList<String>();
    private CopyElfs ce;
    private String mmcblkpath="";
    private boolean IsSuccess=false;
    private String Size="";
    private String IOExceptions="";
    private Button startTest;
    private Button FrequencyButton;
    private Button ModelButton;
    private TextView ModelText;
    private TextView Textinf;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_r_w_test);
        ce = new CopyElfs(getBaseContext());
        ce.copyAll2Data();
        startTest=(Button)findViewById(R.id.startTest);
        FrequencyText=(TextView)findViewById(R.id.FrequencyText);
        ModelButton=(Button)findViewById(R.id.ModelButton);
        ModelText=(TextView)findViewById(R.id.ModelText);
        FrequencyButton=(Button)findViewById(R.id.FrequencyButton);
        Textinf=(TextView)findViewById(R.id.testinf);
        Intent intent=this.getIntent();
        mmcblkpath=intent.getStringExtra("path");
        Toast.makeText(RWTest.this,mmcblkpath,Toast.LENGTH_LONG).show();
    }
    private void startRWTest() throws InterruptedException {
        if(mmcblkpath.isEmpty()){
            Toast.makeText(this,"路径为空，不能测试",Toast.LENGTH_SHORT).show();
            return ;
        }
        IsSuccess=false;
        String stf=String.valueOf(TestFrequency);
        String cmd="do_test "+mmcblkpath+" "+TestModel+" "+stf;
        Toast.makeText(this,cmd,Toast.LENGTH_SHORT).show();
        buffer= ce.callElf(cmd);
        if(buffer!=null)
        {
            String text="";
            for(int i=0;i<buffer.size();i++){
                text+=buffer.get(i).toString()+"\n";
            }
            if(text!=null){
                Textinf.setText(text);
            }
        }
    }
    private String[] list_to_array() {
        IOExceptions = "";
        List<String> list = new ArrayList<String>();
        for (int i = 0; i < buffer.size(); i++) {
            if(buffer.get(i).startsWith("/dev/block"))
                list.add(buffer.get(i));
        }
        String[] array = new String[list.size()];
        for (int i = 0; i < list.size(); i++) {
            array[i] = String.valueOf(list.get(i));
        }
        buffer.clear();
        return array;
    }
    public void myClick(View v) throws InterruptedException {
        switch (v.getId()){
            case R.id.FrequencyButton:
                ShowChooseFrequency();
                break;
            case R.id.ModelButton:
                ShowChooseModel();
                break;
            case R.id.startTest:
                startRWTest();
                break;

        }
        // startActivity(intent);
    }
    private void ShowChooseFrequency(){
        AlertDialog.Builder dialog = new AlertDialog.Builder (this);
        dialog.setTitle ("选择测试次数");
        dialog.setSingleChoiceItems(TestFrequencyArray, 0, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Toast.makeText(RWTest.this,TestFrequencyArray[which], Toast.LENGTH_SHORT).show();
                TestFrequency=TestFrequencyArray[which];
            }
        });
        dialog.setPositiveButton("确定", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
                FrequencyText.setText("你选择的测试次数为 ： "+TestFrequency+"");
            }
        });

        // show()方法显示对话框
        dialog.show ();
    }
    private void ShowChooseModel(){
        AlertDialog.Builder dialog = new AlertDialog.Builder (this);
        dialog.setTitle ("选择测试模式");
        dialog.setSingleChoiceItems(TestModelArray, 0, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Toast.makeText(RWTest.this,TestModelArray[which], Toast.LENGTH_SHORT).show();
                TestModel=TestModelArray[which];
            }
        });
        dialog.setPositiveButton("确定", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
                ModelText.setText("你选择的测试模式为 ： "+TestModel+"");
            }
        });

        // show()方法显示对话框
        dialog.show ();
    }
}