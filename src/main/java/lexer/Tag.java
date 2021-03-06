package lexer;

import java.lang.reflect.Field;

/**
 * Created by sulvto on 16-12-8.
 */
public class Tag {
    public static final int ID = 256, AND = 257, EQ = 258, OR = 259,
            NE = 260, GE = 261, LE = 262, TRUE = 263, FALSE = 264,
            VOID = 265, CHAR = 266, SHORT = 267, UNION = 268,
            ENUM = 269, STATIC = 270, EXTERN = 271, CONST = 272,
            SIGNED = 273, UNSIGNED = 274, IF = 275, ELSE = 276,
            SWITCH = 277, CASE = 278, DEFAULT = 279, WHILE = 280,
            DO = 281, FOR = 282, RETURN = 283, BREAK = 284,
            CONTINUE = 285, GOTO = 286, TYPEDEF = 287, IMPORT = 288,
            SIZEOF = 289, NUM = 290, REAL = 291, STRUCT = 292,
            CHARACTER = 293, STRING = 294, INT = 295, FLOAT = 296,
            BOOL = 297, LONG = 298, EOF = 299;


    public static String string(int tag) {
        Field[] fields = Tag.class.getFields();

        try {
            for (int i = 0; i < fields.length; i++) {
                if (fields[i].getInt(null) == tag) {
                    return fields[i].getName();
                }
            }
            char[] a = {(char) tag};
            return new String(a);
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        }
    }
}
