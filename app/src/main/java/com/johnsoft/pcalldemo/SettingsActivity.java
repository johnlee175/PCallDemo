package com.johnsoft.pcalldemo;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

public class SettingsActivity extends AppCompatActivity {
    private TextView v2;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);
        v2 = findViewById(R.id.label);
        findViewById(R.id.btn).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(final View v) {
                doSomething();
                colorIt();
            }
        });
    }

    public void setText(final String text) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ((TextView) findViewById(R.id.tv)).setText(text);
            }
        });
    }

    public void doSomething() {
        v2.setText(getString());
    }

    public String getString() {
        return "lzh30002995ss";
    }

    public void colorIt() {
        System.out.println("color it color it color it color it ");
    }
}
