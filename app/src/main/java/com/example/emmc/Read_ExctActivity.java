package com.example.emmc;
import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.method.ScrollingMovementMethod;
import android.text.style.ForegroundColorSpan;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;


public class Read_ExctActivity extends AppCompatActivity {

    private static final String TAG ="111" ;
    //导入操作
    private CopyElfs ce;
    // 可执行文件mmc的路径
    private  String mmcpath;


    static final int SUCCESS = 1;
    private static boolean mHaveRoot = false;
    private Process p;
    //文件操作
    private TextView chip_filePah;
    private Button chip_btnSelect;
    //
    private Button mBtnEnter;
    private Button  mBtnClear;
    private TextView tv;
    private String devicepath;
    //导入操作
    // UI线程消息队列
    static final int NORMAL = 1;
    static final int ERROR = 2;
    private Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {//此方法在ui线程运行
            String tCmd = null;
            switch (msg.what) {
                case NORMAL:
                    super.handleMessage(msg);

                    Bundle tBundle = msg.getData();
                    tCmd = tBundle.getString("OUT");
                    tv.append(tCmd);
                    break;
                case ERROR:
                    super.handleMessage(msg);
                    Bundle tBundle2 = msg.getData();
                    SpannableString spanString = new SpannableString(tBundle2.getString("ERROR"));
                    //再构造一个改变字体颜色的Span, 错误反馈设置为红色
                    ForegroundColorSpan span = new ForegroundColorSpan(Color.RED);
                    //将这个Span应用于指定范围的字体
                    spanString.setSpan(span, 0, spanString.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                    tv.append(spanString);
                    break;
                default:
                    break;
            }
        }
    };

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_read);
        tv = (TextView)findViewById(R.id.sample_text);
        tv.setMovementMethod(ScrollingMovementMethod.getInstance());
        // 清空textview
        tv.setText(null);

        // 创建可执行文件类，并用此类执行mmc可执行文件
        ce = new CopyElfs(getBaseContext());
        ce.copyAll2Data();
        Bundle bundle = this.getIntent().getExtras();
        devicepath=bundle.getString("IC_path");
        Log.e(TAG, devicepath);
        //可执行文件路径
        mmcpath = ce.getExecutableFilePath()+"/mmc extcsd read ";
        if (!devicepath.isEmpty()) {
            String seleted_cmd = mmcpath + devicepath;
            try {
                ce.callElf(seleted_cmd);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }



}
