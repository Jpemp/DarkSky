package com.darksky.services;

import com.darksky.models.User;
import com.darksky.utils.DatabaseConnection;
import org.mindrot.jbcrypt.BCrypt;

import java.sql.*;

public class UserService {
    private Connection connection;

    public UserService(Connection connection) {
        this.connection = connection;
    }

    public User authenticateUser(String userid, String password) {
        String sql = "SELECT * FROM users WHERE userid = ?";
        try (PreparedStatement stmt = connection.prepareStatement(sql)) {
            stmt.setString(1, userid);
            ResultSet rs = stmt.executeQuery();

            if (rs.next()) {
                String userId = rs.getString("userid");
                String name = rs.getString("name");
                String role = rs.getString("role");
                String email = rs.getString("email");
                String hashedPassword = rs.getString("password");

                if (BCrypt.checkpw(password, hashedPassword)) {
                    return new User(userId, name, role, email, hashedPassword, true);
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return null;
    }

    public boolean addUser(User user) {
        String sql = "INSERT INTO users (userid, name, email, password, role) VALUES (?, ?, ?, ?, ?)";
        try (Connection conn = DatabaseConnection.getConnection();
             PreparedStatement stmt = conn.prepareStatement(sql)) {

            String hashedPassword = user.getPassword();
            if (!hashedPassword.startsWith("$2a$")) {
                hashedPassword = BCrypt.hashpw(hashedPassword, BCrypt.gensalt());
            }

            stmt.setString(1, user.getUserid());
            stmt.setString(2, user.getName());
            stmt.setString(3, user.getEmail());
            stmt.setString(4, hashedPassword);
            stmt.setString(5, user.getRole());

            int rowsAffected = stmt.executeUpdate();
            return rowsAffected > 0;

        } catch (SQLException e) {
            System.err.println("Error adding user: " + e.getMessage());
            return false;
        }
    }

    public boolean changePassword(String userid, String newPassword) {
        String sql = "UPDATE users SET password = ? WHERE userid = ?";
        try (Connection conn = DatabaseConnection.getConnection();
             PreparedStatement stmt = conn.prepareStatement(sql)) {

            String hashedPassword = BCrypt.hashpw(newPassword, BCrypt.gensalt());
            stmt.setString(1, hashedPassword);
            stmt.setString(2, userid);

            int rowsAffected = stmt.executeUpdate();
            return rowsAffected > 0;

        } catch (SQLException e) {
            e.printStackTrace();
        }
        return false;
    }

    public String getAllUsers() {
        StringBuilder usersList = new StringBuilder();

        try {
            String query = "SELECT name, userid, email, role FROM users";
            Statement statement = connection.createStatement();
            ResultSet resultSet = statement.executeQuery(query);

            while (resultSet.next()) {
                String username = resultSet.getString("name");
                String userid = resultSet.getString("userid");
                String email = resultSet.getString("email");
                String role = resultSet.getString("role");

                if (email == null) {
                    email = "No Email";
                }

                usersList.append("Name: ").append(username)
                        .append(", ID: ").append(userid)
                        .append(", Email: ").append(email)
                        .append(", Role: ").append(role)
                        .append("\n");
            }

            resultSet.close();
            statement.close();

        } catch (SQLException e) {
            e.printStackTrace();
        }

        return usersList.toString();
    }

    public User getUserById(String userId) {
        String sql = "SELECT * FROM users WHERE userid = ?";
        try (Connection conn = DatabaseConnection.getConnection();
             PreparedStatement stmt = conn.prepareStatement(sql)) {

            stmt.setString(1, userId);
            ResultSet rs = stmt.executeQuery();

            if (rs.next()) {
                String name = rs.getString("name");
                String role = rs.getString("role");
                String email = rs.getString("email");
                String password = rs.getString("password");

                return new User(userId, name, role, email, password, true);
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return null;
    }

    public boolean deleteUser(String userId) {
        String sql = "DELETE FROM users WHERE userid = ?";
        try (Connection conn = DatabaseConnection.getConnection();
             PreparedStatement stmt = conn.prepareStatement(sql)) {

            stmt.setString(1, userId);

            int rowsAffected = stmt.executeUpdate();
            return rowsAffected > 0;

        } catch (SQLException e) {
            e.printStackTrace();
        }
        return false;
    }
}
