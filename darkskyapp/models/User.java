package com.darksky.models;

import org.mindrot.jbcrypt.BCrypt;

public class User {
    private String userid;
    private String name;
    private String role;
    private String email;
    private String password;

    public User(String userid, String name, String role, String email, String password, boolean isHashed) {
        this.userid = userid;
        this.name = name;
        this.role = role;
        this.email = email;
        this.password = isHashed ? password : hashPassword(password);
    }

    public String getUserid() {
        return userid;
    }

    public String getName() {
        return name;
    }

    public String getRole() {
        return role;
    }

    public String getEmail() {
        return email;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(String password) {
        this.password = hashPassword(password);
    }

    public boolean verifyPassword(String plainPassword) {
        return BCrypt.checkpw(plainPassword, this.password);
    }

    private static String hashPassword(String password) {
        if (password.startsWith("$2a$")) {
            return password;
        }
        return BCrypt.hashpw(password, BCrypt.gensalt());
    }
}
