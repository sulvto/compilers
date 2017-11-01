package ast;

import lexer.Token;

import java.io.PrintStream;
import java.util.List;

/**
 * Created by sulvto on 16-12-8.
 */
public class Dumper {
    private int nIndex;
    private PrintStream stream;

    public Dumper(PrintStream printStream) {
        this.stream = printStream;
        this.nIndex = 0;
    }

    void indent() {
        nIndex++;
    }

    void unIndent() {
        nIndex--;
    }

    private static final String indentString = "    ";

    private void printIndent() {
        int n = nIndex;
        while (n > 0) {
            stream.print(indentString);
            n--;
        }
    }


    void printPair(String name, String value) {
        printIndent();
        stream.println(name + ": " + value);
    }

    public void printMember(String name, Dumpable d) {
        if (d == null) {
            printPair(name, "null");
        } else {
            printIndent();
            stream.println(name + ":");
            indent();
            d.dump(this);
            unIndent();
        }
    }

    public void printMember(String name, List<? extends Dumpable> nodes) {
        printIndent();
        stream.println(name + ":");
        indent();
        for (Dumpable n : nodes) {
            n.dump(this);
        }
        unIndent();
    }


    public void printClass(Object obj, Token token) {
        printIndent();
        stream.println("<<" + obj.getClass().getSimpleName() + ">>(" + token + ")");
    }

    public void printMember(String name, String str) {
        printPair(name,str);
    }

    public void printMember(String name, long value) {
        printPair(name, value + "");
    }
}
