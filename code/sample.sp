import people for People
fun fn(){
    var p = People.new("xiaoming","male")
    p.sayHi()
}

class Family < People{
    var father
    var mother
    var child
//    var age = 123
    new(f,m,c){
        father = f
        mother = m
        child = c
        super("wbf","male")
    }

}

var f = Family.new("wbf","ls","shine")
f.sayHi()

fn()