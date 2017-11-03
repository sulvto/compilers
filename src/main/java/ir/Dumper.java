package ir;

import java.io.PrintStream;
import java.util.List;

/**
 * Created by sulvto on 17-11-4.
 */
public class Dumper {
    final PrintStream stream;
    private int numIndent;

    Dumper(PrintStream stream) {
        this.stream = stream;
        numIndent = 0;
    }

    public void printClass(Object object) {
        printIndent();
        stream.println("<<" + object.getClass().getSimpleName() + ">>");
    }

    public void printClass(Object object, String location) {
        printIndent();
        stream.println("<<" + object.getClass().getSimpleName() + ">>" + "(" + location + ")");
    }

    public void printMember(String name, int memb) {
        printPair(name, "" + memb);
    }


    public void printMember(String name, long memb) {
        printPair(name, "" + memb);
    }


    public void printMember(String name, boolean memb) {
        printPair(name, "" + memb);
    }


    public void printMember(String name, String memb) {
        printPair(name, "" + memb);
    }


    public void printMember(String name, LabelStmt memb) {
        printPair(name, "" + memb);
    }


    public void printMember(String name, asm.Type memb) {
        printPair(name, "" + memb.toString());
    }


    public void printMember(String name, type.Type memb) {
        printPair(name, "" + memb.toString());
    }

    private void printPair(String name, String value) {
        printIndent();
        stream.println(name + ":" + value);
    }

    public void printMember(String name, Dumpable memb) {
        printIndent();
        if (memb == null) {
            stream.println(name + ":null");
        } else {
            stream.println(name + ":");
            indent();
            memb.dump(this);
            unindent();
        }
    }


    public void printMember(String name, List<? extends Dumpable> elems) {
        printIndent();
        stream.println(name + ":");
        indent();
        elems.forEach(elem -> elem.dump(this));
        unindent();
    }

    private void unindent() {
        numIndent--;
    }

    private void indent() {
        numIndent++;
    }

    final static private String indentString = "    ";

    private void printIndent() {
        int n = numIndent;
        while (n > 0) {
            stream.print(indentString);
            n--;
        }
    }
}
