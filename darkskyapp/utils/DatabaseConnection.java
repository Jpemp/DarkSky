package com.darksky.utils;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;

public class DatabaseConnection {
    private static String URL;
    private static String USER;
    private static String PASSWORD;

    static {
        try {
            String appDir = System.getProperty("user.dir");
            File configFile = new File(appDir, "config.properties");

            if (configFile.exists()) {
                Properties properties = new Properties();
                properties.load(new FileInputStream(configFile));

                URL = properties.getProperty("db.url");
                USER = properties.getProperty("db.user");
                PASSWORD = properties.getProperty("db.password");
            } else {
                System.out.println("Error: config.properties not found at: " + configFile.getAbsolutePath());
            }

        } catch (IOException e) {
            e.printStackTrace();
            System.out.println("Error loading config file!");
        }
    }

    public static Connection getConnection() {
        try {
            return DriverManager.getConnection(URL, USER, PASSWORD);
        } catch (SQLException e) {
            e.printStackTrace();
            System.out.println("Error trying to connect to database.");
            return null;
        }
    }
}
