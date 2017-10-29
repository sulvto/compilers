import java.util.ArrayList;
import java.util.List;

/**
 * Created by sulvto on 17-10-26.
 */
public class PieAST {
    Token token;
    Scope scope;
    private List<PieAST> children;

    public PieAST(Token token) {
        this.token = token;
    }

    public PieAST(int tokenType) {
        this.token = new Token(tokenType);
    }


    public int getType() {
        return token.type;
    }

    public String getText() {
        return token.text;
    }


    public void addChild(PieAST child) {
        if (children == null) children = new ArrayList<>();
         children.add(child);
    }

    public PieAST getChild(int i) {
        return children.get(i);
    }

    public boolean isNil() {
        return token == null;
    }

    public List<PieAST> getChildren() {
        return children;
    }

    public int getChildCount() {
        return children.size();
    }

    @Override
    public String toString() {
        return token != null ? token.toString() : "nil";
    }

    public String toStringTree() {
        if (children == null || children.size() == 0) return this.toString();
        StringBuilder builder = new StringBuilder();
        if (!isNil()) {
            builder.append("(");
            builder.append(this.toString());
            builder.append(' ');
        }
        for (int i = 0; i < children.size(); i++) {
            PieAST ast = children.get(i);
            if (i > 0) builder.append(' ');
            builder.append(ast.toStringTree());
        }
        if (!isNil()) builder.append(")");
        return builder.toString();
    }
}
