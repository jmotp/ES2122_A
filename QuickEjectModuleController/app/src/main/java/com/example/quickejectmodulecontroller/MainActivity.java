package com.example.quickejectmodulecontroller;

import static java.lang.String.format;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.net.Uri;
import android.os.CountDownTimer;
import android.os.Handler;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebView;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.UUID;

import com.loopj.android.http.*;

import cz.msebera.android.httpclient.Header;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {
    Button startButton, updateButton, infoButton;
    TextView timerView;
    EditText timerCount, depthRef;
    RadioGroup timerUnitGroup, operator;
    Spinner firstMode, secondMode;
    WebView urlSender;
    Integer countS, countU;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        firstMode = findViewById(R.id.opMode);
        secondMode = findViewById(R.id.opMode2);

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(MainActivity.this,
                android.R.layout.simple_list_item_1, getResources().getStringArray(R.array.mode_types)){
            @Override
            public boolean isEnabled(int position) {
                if (position == 0) {
                    // Disable the first item from Spinner
                    // First item will be use for hint
                    return false;
                } else {
                    return true;
                }
            }

            @Override
            public View getDropDownView(int position, View convertView,
                                        ViewGroup parent) {
                View view = super.getDropDownView(position, convertView, parent);
                TextView tv = (TextView) view;
                if (position == 0) {
                    // Set the hint text color gray
                    tv.setTextColor(Color.GRAY);
                } else {
                    tv.setTextColor(Color.BLACK);
                }
                return view;
            }

        };
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        firstMode.setAdapter(adapter);
        secondMode.setAdapter(adapter);


        timerCount = findViewById(R.id.time);
        depthRef = findViewById(R.id.depth);
        updateButton = findViewById(R.id.updateButton);
        startButton = findViewById(R.id.startButton);
        infoButton = findViewById(R.id.infoButton);
        timerView = findViewById(R.id.timerCountDown);
        timerUnitGroup = findViewById(R.id.timerUnit);
        operator = findViewById(R.id.modeOperator);
        urlSender = findViewById(R.id.webview);

        updateButtonClick();
        startButtonClick();
        infoButtonClick();
    }

    private void infoButtonClick() {
        infoButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                final Handler handlerInfo = new Handler();

                Thread update = new Thread(){
                    @Override
                    public void run() {

                        Log.i("Thread Fetch Running","Running");
                        try {
                            URL url = new URL("http://192.168.4.1/info");

                            HttpURLConnection urlConnection = (HttpURLConnection) url.openConnection();
                            InputStream in = new BufferedInputStream(urlConnection.getInputStream());

                            BufferedReader reader = new BufferedReader(new InputStreamReader(in));

                            String data = reader.readLine();

                            final JSONObject jsonObject = new JSONObject(data);

                            handlerInfo.post(new Runnable() {
                                @Override
                                public void run() {
                                    try {
                                        // check if update successful
                                        handleInfo(jsonObject.getString("status"));
                                    } catch (JSONException e) {
                                        e.printStackTrace();
                                    }
                                }
                            });
                        } catch (MalformedURLException e) {
                            e.printStackTrace();
                        } catch (JSONException e) {
                            e.printStackTrace();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                };
                update.start();

            }
        });
    }

    private void startButtonClick() {
        startButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                /*Send data via HTTP GET*/
                String urlStr = "http://192.168.4.1/start";

                final Handler handlerStart = new Handler();

                Thread startT = new Thread(){
                    @Override
                    public void run() {

                        Log.i("Thread Start Running","Running");
                        try {
                            URL url = new URL(urlStr);

                            HttpURLConnection urlConnection = (HttpURLConnection) url.openConnection();
                            InputStream in = new BufferedInputStream(urlConnection.getInputStream());

                            BufferedReader reader = new BufferedReader(new InputStreamReader(in));

                            String data = reader.readLine();

                            final JSONObject jsonObject = new JSONObject(data);

                            handlerStart.post(new Runnable() {
                                @Override
                                public void run() {
                                    try {
                                        // check if start successful
                                        handleStart(jsonObject.getString("status"));
                                    } catch (JSONException e) {
                                        e.printStackTrace();
                                    }
                                }
                            });
                        } catch (MalformedURLException e) {
                            e.printStackTrace();
                        } catch (JSONException e) {
                            e.printStackTrace();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                };
                startT.start();

            }
        });
    }

    private void updateButtonClick() {
        updateButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                String urlStr = createUpdateUrl();

                if (urlStr != null) {

                    final Handler handlerUpdate = new Handler();

                    Thread update = new Thread(){
                        @Override
                        public void run() {

                            Log.i("Thread Update Running","Running");
                            try {
                                URL url = new URL(urlStr);

                                HttpURLConnection urlConnection = (HttpURLConnection) url.openConnection();
                                InputStream in = new BufferedInputStream(urlConnection.getInputStream());

                                BufferedReader reader = new BufferedReader(new InputStreamReader(in));

                                String data = reader.readLine();

                                final JSONObject jsonObject = new JSONObject(data);

                                handlerUpdate.post(new Runnable() {
                                    @Override
                                    public void run() {
                                        try {
                                            // check if update successful
                                            handleUpdate(jsonObject.getString("status"));
                                        } catch (JSONException e) {
                                            e.printStackTrace();
                                        }
                                    }
                                });
                            } catch (MalformedURLException e) {
                                e.printStackTrace();
                            } catch (JSONException e) {
                                e.printStackTrace();
                            } catch (IOException e) {
                                e.printStackTrace();
                            }
                        }
                    };
                    update.start();
                }
            }
        });
    }

    private String createUpdateUrl() {
        Integer mode1, mode2, timer, depth;
        mode1 = getSelectedMode(firstMode);
        mode2 = getSelectedMode(secondMode);
        if ((mode1 == 0 && mode2 == 0)){
            return null;
        }
        else{
            String url;
            if(mode2 == 0){
                url = "http://192.168.4.1/get?mode1="+mode1+"&op=NULL&mode2=0";
            }
            else if(mode1 == 0){
                url = "http://192.168.4.1/get?mode1=0&op=NULL&mode2="+mode2;
            }
            else{
                String op = ((RadioButton)findViewById(timerUnitGroup.getCheckedRadioButtonId()))
                        .getText().toString();
                if (op == null) {
                    return null;
                }
                url = "http://192.168.4.1/get?mode1="+mode1+"&op="+op+"&mode2="+mode2;
            }


            timer = getTimerValue();
            if(timer > 0 && (mode1 == 1 || mode2 == 1)){
                url = url + "&timer="+timer;
            }
            else if(timer <= 0 && (mode1 == 1 || mode2 == 1)){
                return null;
            }
            else{
                url = url + "&timer=0";
            }

            depth = getDepthValue();
            if(depth > 0 && (mode1 == 2 || mode2 == 2)){
                url = url + "&depth="+depth;
            }
            else if(depth <= 0 && (mode1 == 2 || mode2 == 2)){
                return null;
            }
            else{
                url = url + "&depth=0";
            }

            return url;
        }
    }

    private Integer getDepthValue() {
        String aux = depthRef.getText().toString();
        if(aux == null)
            return 0;
        else
            return Integer.parseInt(aux);
    }

    private void handleStart(String check) {
        if (check.equals("successful")){
            if (getSelectedMode(firstMode) == 1 || getSelectedMode(secondMode) == 1){
                setCountDown(true, getTimerValue());
            }
            else {
                setCountDown(false, 5);
            }
            updateButton.setEnabled(false);
            startButton.setEnabled(false);
            Toast.makeText(getApplicationContext(), "Module Timer started successfully", Toast.LENGTH_LONG).show();
        }
        else{
            Toast.makeText(getApplicationContext(), "Module Timer failed to start", Toast.LENGTH_LONG).show();
        }
    }

    private void handleUpdate(String check){
        if (check.equals("successful")){
            Toast.makeText(getApplicationContext(), "Module updated successfully", Toast.LENGTH_LONG).show();
        }
        else{
            Toast.makeText(getApplicationContext(), "Module failed to update", Toast.LENGTH_LONG).show();
        }
    }


    private void handleInfo(String status) {
        if (status.equals("unsuccessful")){
            Toast.makeText(getApplicationContext(), "Failed to connect to Module and Fetch info", Toast.LENGTH_LONG).show();
        }
        else{
            // set value of app to values of Module
            // set first operating mode
            int mode1 = status.charAt(0) - '0';
            firstMode.setSelection(mode1);

            // set the operator
            int op = status.charAt(2) - '0';
            operator.clearCheck();
            if(op == 1){
                operator.check(R.id.radioAnd);
            }
            else if(op == 2){
                operator.check(R.id.radioOr);
            }

            // set second operating mode
            int mode2 = status.charAt(4) - '0';
            secondMode.setSelection(mode2);

            // set the timer unit and its value
            int time = Integer.parseInt(status.substring(6));
            timerUnitGroup.clearCheck();
            if((time%360) == 0){
                time = time/360;
                timerUnitGroup.check(R.id.radioHours);
            }
            else if((time%60)==0){
                time = time/60;
                timerUnitGroup.check(R.id.radioMinutes);
            }
            else{
                timerUnitGroup.check(R.id.radioSeconds);
            }
            timerCount.setText(Integer.toString(time));

        }
    }

    private Integer getSelectedMode(Spinner mode) {
        String mode1 = mode.getSelectedItem().toString();
        // get selected mode
        if (mode1.equals("Timer Mode")) {
            return 1;
        }
        else if (mode1.equals("Depth Mode")) {
            return 2;
        }
        else if (mode1.equals("Battery Mode")) {
            return 3;
        }
        else{
            return 0;
        }
    }

    private Integer getTimerValue() {
        Integer timer = Integer.parseInt(timerCount.getText().toString());
        String timeUnit = ((RadioButton)findViewById(timerUnitGroup.getCheckedRadioButtonId()))
                .getText().toString();
        if(timeUnit == null || timer == null){
            return -1;
        }
        switch (timeUnit) {
            case "seconds":
                timer = timer * 1;
                break;
            case "minutes":
                timer = timer * 60;
                break;
            case "hours":
                timer = timer * 60 * 60;
                break;
        }

        return timer;
    }

    private void setCountDown(boolean timerMode, Integer timer) {
        // update countdown timer if at least one of the modes is timer
        // else disables the button for 10 seconds so they don't update while the mission is starting
        new CountDownTimer(timer*1000, 1000) {
            public void onTick(long millisUntilFinished) {
                if(timerMode) {
                    Long seconds, minutes, hours;
                    seconds = (millisUntilFinished / 1000);
                    minutes = seconds / 60;
                    seconds = seconds % 60;
                    hours = minutes / 60;
                    minutes = minutes % 60;


                    timerView.setText(format("%02d", hours) + ":" +
                            format("%02d", minutes) + ":" +
                            format("%02d", seconds));
                    if (millisUntilFinished < 10000) {
                        timerView.setTextColor(Color.RED);
                    } else {
                        timerView.setTextColor(Color.BLACK);
                    }
                }
            }

            public void onFinish() {
                if(timerMode) {
                    timerView.setText("EJECT!!!");
                    timerView.setTextColor(Color.GREEN);
                }
                startButton.setEnabled(true);
                updateButton.setEnabled(true);
            }
        }.start();

    }

    @Override
    public void onClick(View view) {

    }

}