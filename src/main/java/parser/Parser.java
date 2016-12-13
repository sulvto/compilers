package parser;

import ast.AST;
import ast.Declarations;
import ast.TypedefNode;
import lexer.Lexer;
import lexer.Tag;
import lexer.Token;

import java.io.IOException;

/**
 * Created by sulvto on 16-12-8.
 */
public class Parser {
    public Lexer lex;
    public Token look;

    public Parser(Lexer lex) throws IOException {
        this.lex = lex;
        move();
    }

    void move() throws IOException {
        look = lex.scan();
    }

    AST compilationUnit() {
        // TODO
        Declarations declarations = top_defs();
        return new AST(declarations);
    }

    Declarations top_defs() {
        Declarations declarations = new Declarations();
        switch (look.tag) {
            case Tag.TYPEDEF:declarations.addTypedef(typedef());break;
            // TODO
        }

        return declarations;
    }

    TypedefNode typedef() {
        // TODO
        return null;
    }

    void match(int tag) throws IOException {
        if (tag == look.tag) move();
        else error("Encountered \"" + (char) look.tag + "\" . Was expecting one of:\"" + (char) tag + "\"");
    }

    private void error(String message) {
        throw new Error("[ERROR] line " + lex.line + ", column " + lex.column + ".\n" + message);
    }
}
