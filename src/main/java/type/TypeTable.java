package type;

import util.ErrorHandler;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by sulvto on 17-11-2.
 */
public class TypeTable {

    private int initSize;
    private int initLong;
    private int pointerSize;
    private Map<TypeRef, Type> table;

    public TypeTable(int initSize, int initLong, int pointerSize) {
        this.table = new HashMap<>();
        this.initSize = initSize;
        this.initLong = initLong;
        this.pointerSize = pointerSize;
    }

    public boolean isDefined(TypeRef ref) {
        return table.containsKey(ref);
    }

    public void put(TypeRef ref, Type type) {
        if (table.containsKey(ref)) {
            throw new Error("duplicated type definition:" + ref);
        }
        table.put(ref, type);
    }


    public Type get(TypeRef ref) {
        return table.get(ref);
    }

    public Type getParamType(TypeRef ref) {
        Type type = get(ref);
        return type.isArray() ? pointerTo(type.baseType()) : type;
    }

    private Type pointerTo(Type baseType) {
        return new PointerType(pointerSize, baseType);
    }

    protected void checkRecursiveDefinition(Type type, ErrorHandler errorHandler) {
        doCheckRecursiveDefinition(type, new HashMap<>(), errorHandler);
    }

    static final protected int checking = 0;
    static final protected int checked = 1;

    private void doCheckRecursiveDefinition(Type type, HashMap<Type, Integer> marks, ErrorHandler errorHandler) {
        if (marks.get(type) == checking) {
            errorHandler.error(type.location(), "recursive type definition: " + type);
            return;
        } else if (marks.get(type) == checked) {
            return;
        } else {
            marks.put(type, checking);
            if (type instanceof CompositeType) {
                CompositeType compositeType = (CompositeType) type;
                doCheckRecursiveDefinition(compositeType.type(), marks, errorHandler);
            } else if (type instanceof ArrayType) {
                ArrayType arrayType = (ArrayType) type;
                doCheckRecursiveDefinition(arrayType.baseType(), marks, errorHandler);
            } else if (type instanceof UserType) {
                UserType userType = (UserType) type;
                doCheckRecursiveDefinition(userType.realType(), marks, errorHandler);
            }
            marks.put(type, checked);
        }
    }
}
