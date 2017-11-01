package ast;

import lexer.Token;

import java.io.PrintStream;

/**
 * Created by sulvto on 16-12-8.
 */
public abstract class Node implements Dumpable {
    public final Token token;

    public Node(Token token) {
        this.token = token;
    }

    protected abstract void doDump(Dumper dumper);

    public void dump() {
        dump(System.out);
    }

    public void dump(PrintStream stream) {
        dump(new Dumper(stream));
    }

    public int type() {
        return token.type;
    }

    @Override
    public void dump(Dumper dumper) {
        dumper.printClass(this, token);
        doDump(dumper);
    }
}
