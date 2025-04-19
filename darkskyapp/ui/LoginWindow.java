package com.darksky.ui;

import com.darksky.imagery.RoundedPanel;
import com.darksky.models.User;
import com.darksky.services.UserService;
import com.darksky.utils.DatabaseConnection;
import com.darksky.imagery.BackgroundPanel;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.net.URL;
import java.sql.Connection;

public class LoginWindow extends JFrame {
    private JTextField useridField;
    private JPasswordField passwordField;
    private JLabel statusLabel;
    private UserService userService;

    public LoginWindow() {
        Connection connection = DatabaseConnection.getConnection();
        userService = new UserService(connection);

        setTitle("DarkSky - Login");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setUndecorated(true);

        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        int screenWidth = screenSize.width;
        int screenHeight = screenSize.height;

        setSize(screenWidth, screenHeight);
        setLocationRelativeTo(null);

        URL backgroundUrl = getClass().getClassLoader().getResource("com/darksky/resources/Shelby-and-Paul-so-cute_WEB.jpg");
        if (backgroundUrl == null) {
            throw new IllegalArgumentException("Background image not found in com.darksky.resources!");
        }
        JPanel panel = new BackgroundPanel(backgroundUrl);

        panel.setLayout(null);
        add(panel);

        addLogoToPanel(panel, screenWidth);

        RoundedPanel formPanel = new RoundedPanel(30);
        formPanel.setLayout(null);
        formPanel.setBackground(new Color(255, 255, 255, 226));

        int panelWidth = (int) (screenWidth * 0.3);
        int panelHeight = (int) (screenHeight * 0.3);
        int panelX = (screenWidth - panelWidth) / 2;
        int panelY = (int) (screenHeight * 0.31);

        formPanel.setBounds(panelX, panelY, panelWidth, panelHeight);
        panel.add(formPanel);

        placeComponents(formPanel, screenWidth);

        setVisible(true);
    }

    private void addLogoToPanel(JPanel panel, int screenWidth) {
        URL logoUrl = getClass().getClassLoader().getResource("com/darksky/resources/blanco-ida.png");
        if (logoUrl == null) {
            throw new IllegalArgumentException("Logo image not found in com.darksky.resources!");
        }
        ImageIcon originalIcon = new ImageIcon(logoUrl);
        Image img = originalIcon.getImage();

        int newWidth = (int) (screenWidth * 0.28);
        int newHeight = (int) (newWidth * 0.54);

        Image resizedImg = img.getScaledInstance(newWidth, newHeight, Image.SCALE_SMOOTH);
        ImageIcon resizedIcon = new ImageIcon(resizedImg);

        JLabel logoLabel = new JLabel(resizedIcon);

        int logoX = (screenWidth - newWidth) / 2;
        logoLabel.setBounds(logoX, 40, newWidth, newHeight);

        panel.add(logoLabel);
        panel.revalidate();
        panel.repaint();
    }

    private void placeComponents(JPanel panel, int screenWidth) {
        JLabel userTitle = new JLabel("Login");
        userTitle.setFont(new Font("Arial", Font.BOLD, 35));
        userTitle.setForeground(Color.BLACK);
        userTitle.setBounds((int) (screenWidth * 0.12), 0, 250, 100);
        panel.add(userTitle);

        JLabel userIdLabel = new JLabel("User ID:");
        userIdLabel.setFont(new Font("Arial", Font.BOLD, 20));
        userIdLabel.setBounds((int) (screenWidth * 0.051), 90, 100, 30);
        panel.add(userIdLabel);

        useridField = new JTextField(20);
        useridField.setFont(new Font("Arial", Font.PLAIN, 18));
        useridField.setBounds((int) (screenWidth * 0.051) + 110, 90, 200, 30);
        panel.add(useridField);

        JLabel passwordLabel = new JLabel("Password:");
        passwordLabel.setFont(new Font("Arial", Font.BOLD, 20));
        passwordLabel.setBounds((int) (screenWidth * 0.051), 140, 100, 30);
        panel.add(passwordLabel);

        passwordField = new JPasswordField(20);
        passwordField.setFont(new Font("Arial", Font.PLAIN, 18));
        passwordField.setBounds((int) (screenWidth * 0.051) + 110, 140, 200, 30);
        panel.add(passwordField);

        JButton loginButton = new JButton("Login");
        loginButton.setFont(new Font("Arial", Font.BOLD, 16));
        loginButton.setBounds((panel.getWidth() - 150) / 2, 190, 150, 40);
        panel.add(loginButton);

        statusLabel = new JLabel("", SwingConstants.CENTER);
        statusLabel.setFont(new Font("Arial", Font.BOLD, 14));
        statusLabel.setForeground(Color.RED);
        statusLabel.setBounds((panel.getWidth() - 300) / 2, 230, 300, 30);
        panel.add(statusLabel);

        loginButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                String userid = useridField.getText();
                String password = new String(passwordField.getPassword());
                new AuthenticateUserTask(userid, password).execute();
            }
        });
    }

    private void openDashboard(User user) {
        JOptionPane.showMessageDialog(this, "Welcome, " + user.getName() + "!", "Login Successful", JOptionPane.INFORMATION_MESSAGE);
        dispose();
        new DashboardWindow(user);
    }

    private class AuthenticateUserTask extends SwingWorker<User, Void> {
        private String userid;
        private String password;

        public AuthenticateUserTask(String userid, String password) {
            this.userid = userid;
            this.password = password;
        }

        @Override
        protected User doInBackground() throws Exception {
            return userService.authenticateUser(userid, password);
        }

        @Override
        protected void done() {
            try {
                User user = get();
                if (user != null) {
                    statusLabel.setText("Welcome, " + user.getName() + "!");
                    openDashboard(user);
                } else {
                    statusLabel.setText("Invalid credentials.");
                }
            } catch (Exception e) {
                statusLabel.setText("An error occurred.");
            }
        }
    }
}
