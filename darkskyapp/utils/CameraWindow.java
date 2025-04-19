package com.darksky.utils;

import org.opencv.core.Mat;
import org.opencv.videoio.VideoCapture;
import org.opencv.videoio.Videoio;
import org.opencv.imgproc.Imgproc;
import org.opencv.imgcodecs.Imgcodecs;

import javax.swing.*;
import java.awt.*;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.BufferedImage;
import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;

public class CameraWindow extends JFrame {
    private JLabel cameraLabel;
    private VideoCapture capture;
    private boolean isRunning;
    private JButton toggleButton, snapshotButton, backButton;
    private JLayeredPane layeredPane;

    public CameraWindow() {
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        int screenWidth = screenSize.width;
        int screenHeight = screenSize.height;

        setTitle("Live Video Feed");
        setUndecorated(true);
        setSize(screenWidth, screenHeight);
        setLocationRelativeTo(null);
        setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

        layeredPane = new JLayeredPane();
        layeredPane.setLayout(null);
        add(layeredPane);

        cameraLabel = new JLabel();
        cameraLabel.setBounds(0, 0, screenWidth, screenHeight);
        cameraLabel.setOpaque(false);
        layeredPane.add(cameraLabel, Integer.valueOf(0));

        toggleButton = createTransparentButton("Stop Camera", screenWidth / 2 - 750, screenHeight / 2 - 425);
        snapshotButton = createTransparentButton("Capture Image", screenWidth / 2 - 615, screenHeight / 2 - 425);
        backButton = createTransparentButton("Back", screenWidth / 2 - 480, screenHeight / 2 - 425);

        layeredPane.add(toggleButton, Integer.valueOf(1));
        layeredPane.add(snapshotButton, Integer.valueOf(1));
        layeredPane.add(backButton, Integer.valueOf(1));

        toggleButton.addActionListener(e -> toggleCamera());
        snapshotButton.addActionListener(e -> captureImage());
        backButton.addActionListener(e -> goBack());

        addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent e) {
                closeWindow();
            }
        });

        startCamera();
        setVisible(true);
    }

    private JButton createTransparentButton(String text, int x, int y) {
        JButton button = new JButton(text);
        button.setBounds(x, y, 125, 25);
        button.setOpaque(false);
        button.setContentAreaFilled(false);
        button.setBorderPainted(true);
        button.setForeground(Color.WHITE);

        button.setBackground(new Color(0, 0, 0, 100));
        return button;
    }

    private void startCamera() {
        isRunning = true;
        capture = new VideoCapture(0);

        double maxWidth = capture.get(Videoio.CAP_PROP_FRAME_WIDTH);
        double maxHeight = capture.get(Videoio.CAP_PROP_FRAME_HEIGHT);

        if (maxWidth < 1920) {
            capture.set(Videoio.CAP_PROP_FRAME_WIDTH, 1920);
            capture.set(Videoio.CAP_PROP_FRAME_HEIGHT, 1080);
        } else {
            capture.set(Videoio.CAP_PROP_FRAME_WIDTH, maxWidth);
            capture.set(Videoio.CAP_PROP_FRAME_HEIGHT, maxHeight);
        }

        System.out.println("Using resolution: " + capture.get(Videoio.CAP_PROP_FRAME_WIDTH) +
                "x" + capture.get(Videoio.CAP_PROP_FRAME_HEIGHT));

        if (!capture.isOpened()) {
            JOptionPane.showMessageDialog(this, "Camera not found!", "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }

        new Thread(() -> {
            Mat frame = new Mat();
            while (isRunning) {
                if (capture.read(frame)) {
                    Imgproc.cvtColor(frame, frame, Imgproc.COLOR_BGR2RGB);
                    updateCameraDisplay(frame);
                }
                try {
                    Thread.sleep(60);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }

    private void stopCamera() {
        isRunning = false;
        if (capture != null) {
            capture.release();
        }
    }

    private void toggleCamera() {
        if (isRunning) {
            stopCamera();
            toggleButton.setText("Start Camera");
        } else {
            startCamera();
            toggleButton.setText("Stop Camera");
        }
    }

    private void captureImage() {
        if (!isRunning || capture == null) return;

        Mat frame = new Mat();
        if (capture.read(frame)) {
            String timestamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
            String filename = timestamp + ".png";

            try {
                // Save the image in the "View Reports" directory
                File reportsDir = new File("View Reports");
                if (!reportsDir.exists()) {
                    if (!reportsDir.mkdirs()) {
                        throw new Exception("Failed to create 'View Reports' directory.");
                    }
                }
                String viewReportsPath = new File(reportsDir, filename).getAbsolutePath();
                System.out.println("Attempting to save to View Reports directory: " + viewReportsPath);
                if (!Imgcodecs.imwrite(viewReportsPath, frame)) {
                    throw new Exception("Failed to save image to 'View Reports' directory.");
                }

                JOptionPane.showMessageDialog(this, "Snapshot saved as " + filename + " in 'View Reports' directory");

            } catch (Exception e) {
                e.printStackTrace();
                JOptionPane.showMessageDialog(this, "Error: " + e.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
            }
        }
    }

    private void goBack() {
        stopCamera();
        setVisible(false);
    }

    private void closeWindow() {
        stopCamera();
        dispose();
    }

    private void updateCameraDisplay(Mat frame) {
        ImageIcon image = new ImageIcon(convertMatToBufferedImage(frame));

        Image scaledImage = image.getImage().getScaledInstance(
                cameraLabel.getWidth(), cameraLabel.getHeight(), Image.SCALE_SMOOTH
        );

        cameraLabel.setIcon(new ImageIcon(scaledImage));
    }

    private BufferedImage convertMatToBufferedImage(Mat mat) {
        int width = mat.width();
        int height = mat.height();
        int channels = mat.channels();
        BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_3BYTE_BGR);
        byte[] sourcePixels = new byte[width * height * channels];
        mat.get(0, 0, sourcePixels);
        image.getRaster().setDataElements(0, 0, width, height, sourcePixels);
        return image;
    }
}
