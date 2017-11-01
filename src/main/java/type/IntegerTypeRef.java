package type;

import lexer.Token;

/**
 * Created by sulvto on 16-12-18.
 */
public class IntegerTypeRef extends TypeRef {
    public static IntegerTypeRef charRef() {
        return new IntegerTypeRef("char");
    }

    public static IntegerTypeRef charRef(Token token) {
        return new IntegerTypeRef(token, "char");
    }

    public static IntegerTypeRef shortRef() {
        return new IntegerTypeRef("short");
    }

    public static IntegerTypeRef shortRef(Token token) {
        return new IntegerTypeRef(token, "short");
    }

    public static IntegerTypeRef intRef() {
        return new IntegerTypeRef("int");
    }

    public static IntegerTypeRef intRef(Token token) {
        return new IntegerTypeRef(token, "int");
    }

    public static IntegerTypeRef longRef() {
        return new IntegerTypeRef("long");
    }

    public static IntegerTypeRef longRef(Token token) {
        return new IntegerTypeRef(token, "long");
    }

    private String name;

    public IntegerTypeRef(String name) {
        super(null);
        this.name = name;
    }

    public IntegerTypeRef(Token token, String name) {
        super(token);
        this.name = name;
    }

    @Override
    public String toString() {
        return name;
    }
}
