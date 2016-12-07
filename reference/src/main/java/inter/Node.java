package inter;

import lexer.Lexer;

/**
 * Created by sulvto on 16-11-6.
 */
public class Node {
    int lexline = 0;

    Node() {
        lexline = Lexer.line;
    }

    void error(String error) {
        throw new Error("near line " + lexline + "： " + error);
    }

    static int labels = 0;

    public int newlabel() {
        return ++lexline;
    }

    public void emitlabel(int i) {
        System.out.print("L" + i + ":");
    }

    public void emit(String s) {
        System.out.println("\t" + s);
    }
}
