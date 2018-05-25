/**
 * Created by sulvto on 17-10-27.
 */
public interface InterpreterListener {
    void info(String msg);

    void error(String msg);

    void error(String msg, Exception e);

    void error(String msg, Token t);
}
