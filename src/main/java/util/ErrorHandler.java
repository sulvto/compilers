package util;

import lexer.Token;

import java.io.PrintStream;

/**
 * Created by sulvto on 17-11-1.
 */
public class ErrorHandler {
    private final String programId;
    protected PrintStream stream;
    private long nError;

    public ErrorHandler(String progId) {
        this.programId = progId;
    }

    public boolean errorOccured() {
        return true;
    }

    public void error(String location, String message) {
        error(location + ":" + message);
    }

    public void error(Token token, String message) {
        error(token.toString(), message);
    }

    public void error(String message) {
        stream.println(programId + ": error:" + message);
        nError++;
    }
}
