
import java.io.IOException;

/**
 * Created by sulvto on 16-11-5.
 */
public class Parser {
    static int lookahead;

    public Parser() throws IOException {
        lookahead = System.in.read();
    }

    void expr() throws IOException {
        term();
        while (true) {
            if (lookahead == '+') {
                match(lookahead);
                term();
                System.out.write('+');
            } else if (lookahead == '-') {
                match(lookahead);
                term();
                System.out.write('-');
            } else {
                return;
            }
        }
    }

    void match(int t) throws IOException {
        if (lookahead == t) {
            lookahead = System.in.read();
        } else {
            throw new Error("syntax error");
        }

    }

    void term() throws IOException {
        if (Character.isDigit(lookahead)) {
            System.out.write(lookahead);
            match(lookahead);
        } else {
            throw new Error("syntax error");
        }
    }
}
