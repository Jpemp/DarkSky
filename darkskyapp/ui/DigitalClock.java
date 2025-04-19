package com.darksky.ui;

import javax.swing.*;
import java.awt.*;
import java.text.SimpleDateFormat;
import java.util.Date;

public class DigitalClock extends JLabel implements Runnable {
    private static final String TIME_FORMAT = "hh:mm a";
    private Thread clockThread;

    public DigitalClock(int screenWidth, int screenHeight) {
        setFont(new Font("Arial", Font.BOLD, 35));
        setForeground(Color.WHITE);
        setHorizontalAlignment(SwingConstants.CENTER);
        setBounds(screenWidth - 250, 50, 200, 40);
        startClock();
    }

    private void startClock() {
        clockThread = new Thread(this);
        clockThread.start();
    }
    @Override
    public void run() {
        while (true) {
            updateClock();
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    private void updateClock() {
        SimpleDateFormat sdf = new SimpleDateFormat(TIME_FORMAT);
        String currentTime = sdf.format(new Date());
        setText(currentTime);
    }
}
