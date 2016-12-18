package type;

import ast.Location;

/**
 * Created by sulvto on 16-12-18.
 */
public class IntegerTypeRef extends TypeRef {
    public static IntegerTypeRef charRef() {
        return new IntegerTypeRef("char");
    }

    public static IntegerTypeRef charRef(Location location) {
        return new IntegerTypeRef(location, "char");
    }

    public static IntegerTypeRef shortRef() {
        return new IntegerTypeRef("short");
    }

    public static IntegerTypeRef shortRef(Location location) {
        return new IntegerTypeRef(location, "short");
    }

    public static IntegerTypeRef intRef() {
        return new IntegerTypeRef("int");
    }

    public static IntegerTypeRef intRef(Location location) {
        return new IntegerTypeRef(location, "int");
    }

    public static IntegerTypeRef longRef() {
        return new IntegerTypeRef("long");
    }

    public static IntegerTypeRef longRef(Location location) {
        return new IntegerTypeRef(location, "long");
    }

    private String name;

    public IntegerTypeRef(String name) {
        super(null);
        this.name = name;
    }

    public IntegerTypeRef(Location location, String name) {
        super(location);
        this.name = name;
    }

    @Override
    public String toString() {
        return name;
    }
}
