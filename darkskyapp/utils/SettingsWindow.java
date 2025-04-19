package com.darksky.utils;

import com.darksky.imagery.BackgroundPanel;
import com.darksky.imagery.RoundedPanel;
import com.darksky.models.User;
import com.darksky.services.UserService;
import com.darksky.ui.DigitalClock;
import com.darksky.ui.DashboardWindow;
import org.mindrot.jbcrypt.BCrypt;

import javax.swing.*;
import java.awt.*;
import java.net.URL;
import java.sql.Connection;

public class SettingsWindow extends JFrame {

    private UserService userService;
    private User currentUser;

    public SettingsWindow(User user) {
        this.currentUser = user;
        Connection connection = DatabaseConnection.getConnection();
        userService = new UserService(connection);

        setTitle("DarkSky - Settings");

        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        int screenWidth = screenSize.width;
        int screenHeight = screenSize.height;

        setSize(screenWidth, screenHeight);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setUndecorated(true);

        URL backgroundUrl = getClass().getClassLoader().getResource("com/darksky/resources/Shelby-and-Paul-so-cute_WEB.jpg");
        if (backgroundUrl == null) {
            throw new IllegalArgumentException("Background image not found in com.darksky.resources!");
        }
        JPanel panel = new BackgroundPanel(backgroundUrl);

        panel.setLayout(null);
        add(panel);

        addLogoToPanel(panel, screenSize.width);

        DigitalClock clock = new DigitalClock(screenSize.width, screenSize.height);
        panel.add(clock);

        RoundedPanel formPanel = new RoundedPanel(30);
        formPanel.setLayout(null);
        formPanel.setBackground(new Color(255, 255, 255, 226));

        int panelWidth = (int) (screenSize.width * 0.25);
        int panelHeight = (int) (screenSize.height * 0.5);
        int panelX = (screenSize.width - panelWidth) / 2;
        int panelY = (int) (screenSize.height * 0.3);
        formPanel.setBounds(panelX, panelY, panelWidth, panelHeight);
        panel.add(formPanel);

        placeComponents(formPanel, screenWidth);

        setLocationRelativeTo(null);
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
        JLabel settingsTitle = new JLabel("Settings");
        settingsTitle.setFont(new Font("Arial", Font.BOLD, 35));
        settingsTitle.setForeground(Color.BLACK);
        settingsTitle.setBounds((int) (panel.getWidth() * 0.32), 10, 250, 100);
        panel.add(settingsTitle);



        int buttonWidth = (int) (screenWidth * 0.15);
        int buttonHeight = 45;
        int startX = (panel.getWidth() - buttonWidth) / 2;
        int startY = 150;

        JButton viewUsersButton = new JButton("View Users");
        viewUsersButton.setFont(new Font("Arial", Font.BOLD, 16));
        viewUsersButton.setBounds(startX, startY - 30, buttonWidth, buttonHeight);
        panel.add(viewUsersButton);

        JButton addUserButton = new JButton("Add Users");
        addUserButton.setFont(new Font("Arial", Font.BOLD, 16));
        addUserButton.setBounds(startX, startY + 30, buttonWidth, buttonHeight);
        panel.add(addUserButton);

        JButton deleteUserButton = new JButton("Delete Users");
        deleteUserButton.setFont(new Font("Arial", Font.BOLD, 16));
        deleteUserButton.setBounds(startX, startY + 90, buttonWidth, buttonHeight);
        panel.add(deleteUserButton);

        JButton backButton = new JButton("Back");
        backButton.setFont(new Font("Arial", Font.BOLD, 16));
        backButton.setBounds(startX, startY + 180, buttonWidth, buttonHeight);
        panel.add(backButton);

        viewUsersButton.addActionListener(e -> {
            String usersList = userService.getAllUsers();
            JOptionPane.showMessageDialog(this, usersList, "Users List", JOptionPane.INFORMATION_MESSAGE);
        });

        addUserButton.addActionListener(e -> {
            if (authenticateAdmin()) {
                String name = JOptionPane.showInputDialog(this, "Enter Name:");
                String password = JOptionPane.showInputDialog(this, "Enter password:");
                String role = JOptionPane.showInputDialog(this, "Enter role (admin/user):");

                if (name != null && password != null && role != null) {
                    String generatedUserId = UserIdGenerator.generateUserId();
                    String hashedPassword = BCrypt.hashpw(password, BCrypt.gensalt()); // Hash the password before creating the user object

                    User newUser = new User(generatedUserId, name, role, "", hashedPassword, true);
                    boolean success = userService.addUser(newUser);

                    if (success) {
                        JOptionPane.showMessageDialog(this, "User added successfully. UserID: " + generatedUserId);
                    } else {
                        JOptionPane.showMessageDialog(this, "Failed to add user.", "Error", JOptionPane.ERROR_MESSAGE);
                    }
                }
            }
        });

        deleteUserButton.addActionListener(e -> {
            if (authenticateAdmin()) {
                String username = JOptionPane.showInputDialog(this, "Enter User ID to delete:");
                if (username != null) {
                    boolean deleted = userService.deleteUser(username);
                    if (deleted) {
                        JOptionPane.showMessageDialog(this, "User deleted successfully.");
                    } else {
                        JOptionPane.showMessageDialog(this, "User not found.");
                    }
                }
            }
        });

        backButton.addActionListener(e -> {
            dispose();
            new DashboardWindow(currentUser);
        });
    }

    private boolean authenticateAdmin() {
        String adminId = JOptionPane.showInputDialog(this, "Enter Admin User ID:");
        String password = JOptionPane.showInputDialog(this, "Enter Admin Password:");

        if (adminId == null || password == null) return false;

        User adminUser = userService.getUserById(adminId);
        if (adminUser != null && "admin".equals(adminUser.getRole()) && adminUser.verifyPassword(password)) {
            return true;
        } else {
            JOptionPane.showMessageDialog(this, "Access Denied! Only admins can perform this action.");
            return false;
        }
    }
}
