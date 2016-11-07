package parser;

import inter.Stmt;
import lexer.Lexer;
import lexer.Token;
import symbols.Env;

import java.io.IOException;

/**
 * Created by sulvto on 16-11-7.
 */
public class Parser {

    private Lexer lex;
    private Token look;
    Env top = null;
    int used = 0;

    public Parser(Lexer l) throws IOException {
        lex = l;
        move();
    }

    void move() throws IOException {
        look = lex.scan();
    }

    void error(String error) {
        throw new Error("near line " + lex.line + " : " + error);
    }

    void match(int t) throws IOException {
        if (look.tag == t) move();
        else error("syntax error");
    }

    public void program() throws IOException {
        Stmt s = block();
        int begin = s.newlabel();
        int after = s.newlabel();
        s.emitlabel(begin);
        s.gen(begin, after);
        s.emitlabel(after);
    }

    Stmt block() throws IOException {
        match('{');
        Env saveEnv = top;
        top = new Env(top);
        decls();
        Stmt s = stmts();
        match('}');
        top = saveEnv;
        return s;

    }

    Stmt stmts() {
        // TODO
        return null;
    }

    void decls() throws IOException {
        // TODO

    }

}
