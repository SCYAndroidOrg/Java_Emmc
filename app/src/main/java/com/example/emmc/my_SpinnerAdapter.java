package com.example.emmc;


import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import androidx.annotation.NonNull;

import java.util.List;

public class my_SpinnerAdapter extends ArrayAdapter<String> {
    Context context;
    List<String> objects;
    public my_SpinnerAdapter(Context context, int textViewResourceId,
                             List<String> items) {
        super(context, textViewResourceId, items);
        this.context = context;
        this.objects = items;
    }
    @Override
    public View getDropDownView(int position, View convertView,
                                ViewGroup parent) {
        // 这个函数修改的是spinner点击之后出来的选择的部分的字体大小和方式
        if (convertView == null) {
            LayoutInflater inflater = LayoutInflater.from(context);
            convertView = inflater.inflate(android.R.layout.simple_spinner_dropdown_item, parent, false);
        }

        //这里使用的text1 直接复制过来就行 不用重新起名 否则可能找不到这个文本框 是系统默认的
        TextView tv = (TextView) convertView.findViewById(android.R.id.text1);
        tv.setText(objects.get(position));
        tv.setTextSize(18);// 这里实现显示文字的设置
        return convertView;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        LayoutInflater inflater = LayoutInflater.from(context);
        convertView = inflater.inflate(android.R.layout.simple_spinner_dropdown_item, parent, false);
        TextView tvgetView=(TextView) convertView.findViewById(android.R.id.text1);
        tvgetView.setText(getItem(position).toString());
        return convertView;
    }

}
