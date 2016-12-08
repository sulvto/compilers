package parser;

import ast.AST;
import lexer.Lexer;
import lexer.Token;

import java.io.IOException;

/**
 * Created by sulvto on 16-12-8.
 */
public class Parser {
    private Lexer lex;
    private Token look;

    public Parser(Lexer lex) throws IOException {
        this.lex = lex;
        move();
    }

    void move() throws IOException {
        look = lex.scan();
    }

    AST compilationUnit() {
        // TODO
        return null;
    }
}
