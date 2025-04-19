package com.darksky.utils;

import javax.swing.*;
import java.awt.*;
import java.io.File;
import java.io.IOException;

public class ViewReports extends JFrame {

    public ViewReports() {
        openViewReportsDirectory();
    }

    private void openViewReportsDirectory() {
        try {
            File reportsDir = new File("View Reports");
            if (reportsDir.exists() && reportsDir.isDirectory()) {
                if (Desktop.isDesktopSupported()) {
                    Desktop.getDesktop().open(reportsDir);
                } else {
                    JOptionPane.showMessageDialog(this, "Desktop operations not supported on this system", "Error", JOptionPane.ERROR_MESSAGE);
                }
            } else {
                JOptionPane.showMessageDialog(this, "No reports found!", "Info", JOptionPane.INFORMATION_MESSAGE);
            }
        } catch (IOException e) {
            e.printStackTrace();
            JOptionPane.showMessageDialog(this, "Error: " + e.getMessage(), "Error", JOptionPane.ERROR_MESSAGE);
        }
    }
}
