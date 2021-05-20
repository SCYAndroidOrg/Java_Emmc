package com.example.emmc;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;




public class HelpActivity extends AppCompatActivity {

    private static final String TAG = "HelpActivity";
    static final int SUCCESS = 1;
    private TextView tv;

    // UI线程消息队列
    private Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {//此方法在ui线程运行
            switch (msg.what) {
                case SUCCESS:
                    super.handleMessage(msg);
                    Bundle tBundle = msg.getData();
                    String tCmd = tBundle.getString("OUT");
                    tv.append(tCmd);
                    break;
                default:
                    break;
            }
        }
    };

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_help);

        tv = (TextView)findViewById(R.id.sample_text);
        tv.setMovementMethod(ScrollingMovementMethod.getInstance()); // 设置textview可上下滑动
        // 清空textview
        tv.setText(null);

        CopyElfs ce = new CopyElfs(getBaseContext());
        ce.copyAll2Data();

        // 生成 Help 命令
        String cmd = ce.getExecutableFilePath() + "/mmc help";
        Log.i(TAG, "exe cmd: " + cmd);
        try {
            ce.callElf(cmd);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
