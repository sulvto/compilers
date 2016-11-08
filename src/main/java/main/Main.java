package main;

import lexer.Lexer;
import parser.Parser;

import java.io.IOException;

/**
 * Created by sulvto on 16-11-7.
 */
public class Main {
    public static void main(String[] args) throws IOException {
        Lexer lexer = new Lexer();
        Parser parser = new Parser(lexer);
        parser.program();
        System.out.write('\n');
    }

    void a(){

    }
}
