import java.util.Random;
import java.util.Scanner;

public class Dotproduct {
    static Random random = new Random();
    static int xorshift32State = (int)System.currentTimeMillis();
    static long xorshiftState = System.currentTimeMillis();
    public static int xorshift32(){
        xorshift32State ^= (xorshift32State << 13);
        xorshift32State ^= (xorshift32State >> 17);
        xorshift32State ^= (xorshift32State << 5);
        return xorshift32State;
    }
    public static long xorshift64() {
        xorshiftState ^= (xorshiftState << 13);
        xorshiftState ^= (xorshiftState >>> 7);
        xorshiftState ^= (xorshiftState << 17);
        return xorshiftState;
    }
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        int size = 0;
        int type = 0;

        System.out.print("Enter the size of the array: \n> ");
        while (true) {
            String line = scanner.nextLine();
            try {
                size = Integer.parseInt(line.trim());
                if (size >= 0) break;
            } catch (NumberFormatException e) {
                System.out.println("Invalid input. Please enter a positive integer: ");
            }
        }
        System.out.println(size);

        System.out.println("Enter the type of the array: ");
        System.out.println("0.XOR Test; 1. int; 2. float; 3. double; 4. signed char; 5. short");
        System.out.print("> ");
        while (true) {
            String line = scanner.nextLine();
            try {
                type = Integer.parseInt(line.trim());
                if (type >= 0 && type <= 5) break;
            } catch (NumberFormatException e) {
                System.out.println("Invalid input, please enter an integer in [0, 5]");
                System.out.print("> ");
            }
        }
        System.out.println("type:" + type);
        for (int i = 0; i < 1; i++) {
            switch (type) {
                case 0:
                    XORSpeedTest(size);
                    break;
                case 1:
                    intDotProduct(size);
                    break;
                case 2:
                    floatDotProduct(size);
                    break;
                case 3:
                    doubleDotProduct(size);
                    break;
                case 4:
                    signedCharDotProduct(size);
                    break;
                case 5:
                    shortDotProduct(size);
                    break;
            }

        }

        scanner.close();
    }

    public static void XORSpeedTest(int size) {
        int[] a = new int[size];
        int[] b = new int[size];
        long sum = 0;

        random.setSeed(System.currentTimeMillis());
        long st = System.nanoTime();
        for (int i = 0; i < size; i++) {
            a[i] = random.nextInt();
            b[i] = random.nextInt();
        }
        long et = System.nanoTime();
        double timeTakenn = (et - st) / 1e9;
        System.out.printf("Generate using random %.6f seconds\n", timeTakenn);

        st = System.nanoTime();
        for (int i = 0; i < size; i++) {
            a[i] =  xorshift32();
            b[i] =  xorshift32();
        }
        et = System.nanoTime();
        double timeTaken = (et - st) / 1e9;
        System.out.printf("Generate using xor %.6f seconds\n", timeTaken);
        System.out.println("Method XOR increase the speed by " + (timeTakenn - timeTaken)/timeTaken*100 + " %.");

    }

    public static void intDotProduct(int size) {
        int[] a = new int[size];
        int[] b = new int[size];
        long sum = 0;

        long st = System.nanoTime();

        for (int i = 0; i < size; i++) {
            a[i] = xorshift32();
            b[i] = xorshift32();
        }
        long et = System.nanoTime();
        double timeTakenn = (et - st) / 1e9;
        System.out.printf("Generate using xor %.6f seconds\n", timeTakenn);


        long startTime = System.nanoTime();
        for (int i = 0; i < size; i++) {
            sum += (long) a[i] * b[i];
        }
        long endTime = System.nanoTime();

        double timeTaken = (endTime - startTime) / 1e9;
        System.out.printf("Dot product: %.6f seconds\n", timeTaken);
        System.out.printf("Dot product: %d\n", sum);
    }

    public static void floatDotProduct(int size) {
        float[] a = new float[size];
        float[] b = new float[size];
        float sum = 0;

        long st = System.nanoTime();

        for (int i = 0; i < size; i++) {
            a[i] = (float) xorshift64();
            b[i] = (float) xorshift64();
        }
        long et = System.nanoTime();
        double timeTakenn = (et - st) / 1e9;
        System.out.printf("Generate using xor %.6f seconds\n", timeTakenn);

        long startTime = System.nanoTime();
        for (int i = 0; i < size; i++) {
            sum += a[i] * b[i];
        }
        long endTime = System.nanoTime();

        double timeTaken = (endTime - startTime) / 1e9;
        System.out.printf("Dot product: %.6f seconds\n", timeTaken);
        System.out.printf("Dot product: %f\n", sum);
    }

    public static void doubleDotProduct(int size) {
        double[] a = new double[size];
        double[] b = new double[size];
        double sum = 0;

        long st = System.nanoTime();

        for (int i = 0; i < size; i++) {
            a[i] = (double) xorshift64();
            b[i] = (double) xorshift64();
        }
        long et = System.nanoTime();
        double timeTakenn = (et - st) / 1e9;
        System.out.printf("Generate using xor %.6f seconds\n", timeTakenn);
        long startTime = System.nanoTime();
        for (int i = 0; i < size; i++) {
            sum += a[i] * b[i];
        }
        long endTime = System.nanoTime();

        double timeTaken = (endTime - startTime) / 1e9;
        System.out.printf("Dot product: %.6f seconds\n", timeTaken);
        System.out.printf("Dot product: %f\n", sum);
    }

    public static void signedCharDotProduct(int size) {
        byte[] a = new byte[size];
        byte[] b = new byte[size];
        long sum = 0;

        long st = System.nanoTime();

        for (int i = 0; i < size; i++) {
            a[i] = (byte) xorshift32();
            b[i] = (byte) xorshift32();
        }
        long et = System.nanoTime();
        double timeTakenn = (et - st) / 1e9;
        System.out.printf("Generate using xor %.6f seconds\n", timeTakenn);

        long startTime = System.nanoTime();
        for (int i = 0; i < size; i++) {
            sum += (long) a[i] * b[i];
        }
        long endTime = System.nanoTime();

        double timeTaken = (endTime - startTime) / 1e9;
        System.out.printf("Dot product: %.6f seconds\n", timeTaken);
        System.out.printf("Dot product: %d\n", sum);
    }

    public static void shortDotProduct(int size) {
        short[] a = new short[size];
        short[] b = new short[size];
        long sum = 0;

        long st = System.nanoTime();
        for (int i = 0; i < size; i++) {
            a[i] = (short) xorshift32();
            b[i] = (short) xorshift32();
        }
        long et = System.nanoTime();
        double timeTakenn = (et - st) / 1e9;
        System.out.printf("Generate using xor %.6f seconds\n", timeTakenn);
        long startTime = System.nanoTime();
        for (int i = 0; i < size; i++) {
            sum += (long) a[i] * b[i];
        }
        long endTime = System.nanoTime();

        double timeTaken = (endTime - startTime) / 1e9;
        System.out.printf("Dot product: %.6f seconds\n", timeTaken);
        System.out.printf("Dot product: %d\n", sum);
    }
}