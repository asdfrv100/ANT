package com.redcarrottt.testapp;

/* Copyright (c) 2017-2018. All rights reserved.
 *  Gyeonghwan Hong (redcarrottt@gmail.com)
 *  Eunsoo Park (esevan.park@gmail.com)
 *  Injung Hwang (sinban04@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import android.Manifest;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.TextView;

import com.redcarrottt.sc.API;
import com.redcarrottt.sc.BTClientAdapter;
import com.redcarrottt.sc.TCPClientAdapter;
import com.redcarrottt.sc.WFDClientAdapter;

public class MainActivity extends AppCompatActivity implements LogReceiver.Callback {
    private static final String kTag = "MainActivity";
    private MainActivity self = this;
    private API mAPI;

    private ReceivingThread mReceivingThread;
    private LogReceiver mBroadcastReceiver;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Logger.setDefaultContext(this);

        this.mBroadcastReceiver = new LogReceiver(this);
        IntentFilter broadcastIntentFilter = new IntentFilter();
        broadcastIntentFilter.addAction(LogReceiver.kAction);
        this.registerReceiver(this.mBroadcastReceiver, broadcastIntentFilter);

        this.requestPermissions();
        this.initializeCommunication();
    }

    private void initializeCommunication() {
        mAPI = API.getInstance();
        TCPClientAdapter tcpClientAdapter = new TCPClientAdapter((short) 2345, "192.168.0.35",
                2345);
        BTClientAdapter btClientAdapter = new BTClientAdapter((short) 3333, "B8:27:EB:77:C3:4A",
                "150e8400-1234-41d4-a716-446655440000");
        WFDClientAdapter wfdClientAdapter = new WFDClientAdapter((short) 3456, 3456, this);

        tcpClientAdapter.set_control_adapter();
        btClientAdapter.set_data_adapter();
        wfdClientAdapter.set_data_adapter();

        this.mReceivingThread = new ReceivingThread();
        this.mReceivingThread.start();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    private void requestPermissions() {
        if (ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.INTERNET) !=
                PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(MainActivity.this, new String[]{Manifest.permission
                    .INTERNET, Manifest.permission.ACCESS_NETWORK_STATE, Manifest.permission
                    .ACCESS_WIFI_STATE, Manifest.permission.CHANGE_WIFI_STATE, Manifest
                    .permission.CHANGE_NETWORK_STATE, Manifest.permission
                    .ACCESS_COARSE_LOCATION}, 0);
        }
    }

    private class ReceivingThread extends Thread {
        private static final String kTag = "Recv";

        public void run() {
            byte[] buf = new byte[100 * 1024 * 1024];
            String sending_buf = "ACK"; /* Ack Message */

            while (true) {
                int receivedLength = mAPI.recv_data(buf, 100 * 1024 * 1024);
                Logger.VERB(kTag, "Received: Size=" + receivedLength);
                int sentLength = mAPI.send_data(sending_buf.getBytes(), sending_buf.length());
                Logger.VERB(kTag, "Sent: Size=" + sentLength);
            }

        }
    }

    @Override
    public void onLogMessage(final int logLevel, final String logMessage) {
        final int kPrintThreshold = LogLevel.VERB;
        final String kTag = "LOG";
        final String kLogMessage = logMessage;
        this.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                String printMessage = kLogMessage;
                switch (logLevel) {
                    case LogLevel.ERR:
                        Log.e(kTag, kLogMessage);
                        printMessage = "[E]" + printMessage;
                        break;
                    case LogLevel.WARN:
                        Log.w(kTag, kLogMessage);
                        printMessage = "[W]" + printMessage;
                        break;
                    case LogLevel.VERB:
                        Log.i(kTag, kLogMessage);
                        printMessage = "[V]" + printMessage;
                        break;
                    case LogLevel.DEBUG:
                        Log.d(kTag, kLogMessage);
                        printMessage = "[D]" + printMessage;
                        break;
                }

                if (logLevel <= kPrintThreshold) {
                    TextView logTextView = (TextView) findViewById(R.id.logTextView);
                    String text = String.valueOf(logTextView.getText());
                    if (text.length() > 10000) {
                        logTextView.setText(printMessage);
                    } else {
                        logTextView.setText(printMessage + "\n" + text);
                    }
                    logTextView.invalidate();
                }
            }
        });
    }
}