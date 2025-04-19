package com.darksky.imagery;

import javax.swing.*;
import java.awt.*;


import javax.swing.*;
import java.awt.*;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import javax.imageio.ImageIO;

public class BackgroundPanel extends JPanel {
    private Image backgroundImage;

    public BackgroundPanel(URL resourceUrl) {
        try {
            if (resourceUrl == null) {
                throw new IllegalArgumentException("Image resource not found.");
            }
            backgroundImage = ImageIO.read(resourceUrl);
        } catch (IOException e) {
            e.printStackTrace();
            throw new IllegalArgumentException("Error loading image.");
        }
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        if (backgroundImage != null) {
            g.drawImage(backgroundImage, 0, 0, getWidth(), getHeight(), this);
        }
    }
}
