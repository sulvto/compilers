package ast;

/**
 * Created by sulvto on 16-12-18.
 */
public class Location {
    private int line;
    private int column;

    public Location(int line, int column) {
        this.line = line;
        this.column = column;
    }

    @Override
    public String toString() {
        return "line:" + line + " column:" + column;
    }
}
