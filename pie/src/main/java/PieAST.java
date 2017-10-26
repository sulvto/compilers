import java.util.List;

/**
 * Created by sulvto on 17-10-26.
 */
public class PieAST {
    private String text;
    private int childCount;

    public int getType() {
        return type;
    }

    public String getText() {
        return text;
    }

    public void setText(String text) {
        this.text = text;
    }

    public PieAST getChild(int i) {
    }

    public List<PieAST> getChildren() {
        return children;
    }

    public int getChildCount() {
        return childCount;
    }
}
