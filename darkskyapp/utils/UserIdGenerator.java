package com.darksky.utils;

import java.util.Random;

public class UserIdGenerator {
    public static String generateUserId() {
        Random random = new Random();
        String letters = "" + (char) ('A' + random.nextInt(26)) +
                (char) ('A' + random.nextInt(26)) +
                (char) ('A' + random.nextInt(26));
        String numbers = String.format("%03d", random.nextInt(1000));
        return (letters + numbers).substring(0, 6);
    }
}
