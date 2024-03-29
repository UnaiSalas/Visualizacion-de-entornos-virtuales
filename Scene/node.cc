#include <cstdio>
#include <cassert>
#include "node.h"
#include "nodeManager.h"
#include "intersect.h"
#include "bboxGL.h"
#include "renderState.h"

using std::string;
using std::list;
using std::map;

// Recipe 1: iterate through children:
//
//    for(list<Node *>::iterator it = m_children.begin(), end = m_children.end();
//        it != end; ++it) {
//        Node *theChild = *it;
//        theChild->print(); // or any other thing
//    }
// 
// Recipe 2: iterate through children inside a const method
//
//    for(list<Node *>::const_iterator it = m_children.begin(), end = m_children.end();
//        it != end; ++it) {
//        const Node *theChild = *it;
//        theChild->print(); // or any other thing
//    }

Node::Node(const string &name) :
	m_name(name),
	m_parent(0),
	m_gObject(0),
	m_light(0),
	m_shader(0),
	m_placement(new Trfm3D),
	m_placementWC(new Trfm3D),
	m_containerWC(new BBox),
	m_checkCollision(true),
	m_isCulled(false),
	m_drawBBox(false) {}

Node::~Node() {
	delete m_placement;
	delete m_placementWC;
	delete m_containerWC;
}

static string name_clone(const string & base) {

	static char MG_SC_BUFF[2048];
	Node *aux;

	int i;
	for(i = 1; i < 1000; i++) {
		sprintf(MG_SC_BUFF, "%s#%d", base.c_str(), i);
		string newname(MG_SC_BUFF);
		aux = NodeManager::instance()->find(newname);
		if(!aux) return newname;
	}
	fprintf(stderr, "[E] Node: too many clones of %s\n.", base.c_str());
	exit(1);
	return string();
}

Node* Node::cloneParent(Node *theParent) {

	string newName = name_clone(m_name);
	Node *newNode = NodeManager::instance()->create(newName);
	newNode->m_gObject = m_gObject;
	newNode->m_light = m_light;
	newNode->m_shader = m_shader;
	newNode->m_placement->clone(m_placement);
	newNode->m_parent = theParent;
	for(list<Node *>::iterator it = m_children.begin(), end = m_children.end();
		it != end; ++it) {
		Node *theChild = *it;
		newNode->m_children.push_back(theChild->cloneParent(this));
	}
	return newNode;
}


Node *Node::clone() {
	return cloneParent(0);
}

///////////////////////////////////
// transformations

void Node::attachGobject(GObject *gobj ) {
	if (!gobj) {
		fprintf(stderr, "[E] attachGobject: no gObject for node %s\n", m_name.c_str());
		exit(1);
	}
	if (m_children.size()) {
		fprintf(stderr, "EW] Node::attachGobject: can not attach a gObject to node (%s), which already has children.\n", m_name.c_str());
		exit(1);
	}
	m_gObject = gobj;
	propagateBBRoot();
}

GObject *Node::detachGobject() {
	GObject *res = m_gObject;
	m_gObject = 0;
	return res;
}

GObject *Node::getGobject() {
	return m_gObject;
}

void Node::attachLight(Light *theLight) {
	if (!theLight) {
		fprintf(stderr, "[E] attachLight: no light for node %s\n", m_name.c_str());
		exit(1);
	}
	m_light = theLight;
}

Light * Node::detachLight() {
	Light *res = m_light;
	m_light = 0;
	return res;
}

void Node::attachShader(ShaderProgram *theShader) {
	if (!theShader) {
		fprintf(stderr, "[E] attachShader: empty shader for node %s\n", m_name.c_str());
		exit(1);
	}
	m_shader = theShader;
}

ShaderProgram *Node::detachShader() {
	ShaderProgram *theShader = m_shader;
	m_shader = 0;
	return theShader;
}

ShaderProgram *Node::getShader() {
	return m_shader;
}

void Node::setDrawBBox(bool b) { m_drawBBox = b; }
bool Node::getDrawBBox() const { return m_drawBBox; }

bool Node::isCulled() const { return m_isCulled; }

void Node::setCheckCollision(bool b) {
	m_checkCollision = b;
	for(list<Node *>::iterator it = m_children.begin(), end = m_children.end();
		it != end; ++it) {
		Node *theChild = *it;
		theChild->setCheckCollision(b);
	}
}

bool Node::getCheckCollision() const { return m_checkCollision; }

///////////////////////////////////
// transformations

void Node::initTrfm() {
	Trfm3D I;
	m_placement->swap(I);
	// Update Geometric state
	updateGS();
}

void Node::setTrfm(const Trfm3D * M) {
	if (!M) {
		fprintf(stderr, "[E] setTrfm: no trfm for node %s\n", m_name.c_str());
		exit(1);
	}
	m_placement->clone(M);
	// Update Geometric state
	updateGS();
}

void Node::addTrfm(const Trfm3D * M) {
	if (!M) {
		fprintf(stderr, "[E] addTrfm: no trfm for node %s\n", m_name.c_str());
		exit(1);
	}
	m_placement->add(M);
	// Update Geometric state
	updateGS();
}

void Node::translate(const Vector3 & P) {
	static Trfm3D localT;
	localT.setTrans(P);
	addTrfm(&localT);
};

void Node::rotateX(float angle ) {
	static Trfm3D localT;
	localT.setRotX(angle);
	addTrfm(&localT);
};

void Node::rotateY(float angle ) {
	static Trfm3D localT;
	localT.setRotY(angle);
	addTrfm(&localT);
};

void Node::rotateZ(float angle ) {
	static Trfm3D localT;
	localT.setRotZ(angle);
	addTrfm(&localT);
};

void Node::scale(float factor ) {
	static Trfm3D localT;
	localT.setScale(factor);
	addTrfm(&localT);
};

///////////////////////////////////
// tree operations

Node *Node::parent() {
	if (!m_parent) return this;
	return m_parent;
}

/**
 * nextSibling: Get next sibling of node. Return first sibling if last child.
 *
 */

Node *Node::nextSibling() {
	Node *p = m_parent;
	if(!p) return this;
	list<Node *>::iterator end = p->m_children.end();
	list<Node *>::iterator it = p->m_children.begin();
	while(it != end && *it != this) ++it;
	assert(it != end);
	++it;
	if (it == end) return *(p->m_children.begin());
	return *it;
}

/**
 * firstChild: Get first child of node. Return itself if leaf node.
 *
 * @param theNode A pointer to the node
 */

Node *Node::firstChild() {
	if (!m_children.size()) return this;
	return *(m_children.begin());
}

// Cycle through children of a Node. Return children[ i % lenght(children) ]

Node * Node::cycleChild(size_t idx) {

	size_t m = idx % m_children.size();
	size_t i = 0;
	for(list<Node *>::iterator it = m_children.begin(), end = m_children.end();
		it != end; ++it) {
		if (i == m) return *it;
		i++;
	}
	return 0;
}

// @@ TODO:
//
// Add a child to node
// Print a warning (and do nothing) if node already has an gObject.

// se mete el hijo en la lista de hijos y con el puntero al padre señalas al padre
void Node::addChild(Node *theChild) {

	if (theChild == 0) return;
	if (m_gObject) {
		// node has a gObject, so print warning
		printf("ESTAS METIENDO UN NODO HIJO EN UN NODO HOJA");
	} else {
		// usar push_back para meter el hijo en la lista de hijos
		m_children.push_back(theChild);
		theChild->m_parent=this;
		// node does not have gObject, so attach child
		updateGS();	
		// hacemos llamada a updateWC();	
	}
}

void Node::detach() {

	Node *theParent;
	theParent = m_parent;
	if (theParent == 0) return; // already detached (or root node)
	m_parent = 0;
	theParent->m_children.remove(this);
	// Update bounding box of parent
	theParent->propagateBBRoot();
}

// @@ TODO: auxiliary function
//
// given a node:
// - update the BBox of the node (updateBB)
// - Propagate BBox to parent until root is reached
//
// - Preconditions:
//    - the BBox of thisNode's children are up-to-date.
//    - placementWC of node and parents are up-to-date

void Node::propagateBBRoot() {

// - update the BBox of the node (updateBB)
	updateBB();
// - Propagate BBox to parent until root is reached
	if (m_parent) 
		m_parent->propagateBBRoot();

	/// !m_parent similar a (m_parent == 0) === No tiene padre === es el nodo raiz

	/// if (m_gObject) === es un nodo hoja OJO CUANDO PREGUNTES SOBRE M_CHILDREN.SIZE() == 0

}

// @@ TODO: auxiliary function
//
// given a node, update its BBox to World Coordinates so that it includes:
//  - the BBox of the geometricObject it contains (if any)
//  - the BBox-es of the children
//
// Note: Bounding box is always in world coordinates
//
// Precontitions:
//
//     * m_placementWC is up-to-date
//     * m_containerWC of children are up-to-date
//
// These functions can be useful
//
// Member functions of BBox class (from Math/bbox.h):
//
//    void init(): Set BBox to be the void (empty) BBox
//    void clone(const BBox * source): Copy from source bounding box
//    void transform(const Trfm3D * T): Transform BBox by applying transformation T
//    void include(BBox *B): Change BBox so that it also includes BBox B
//
// Note:
//    See Recipe 1 in for knowing how to iterate through children.

//Actualizar BB del nodo en el que esta y llama a la funcion con el nodo padre.
// Para cuando ha llegado al nodo raiz


void Node::updateBB () {
		// si es nodo hoja, tiene un objeto geometrico
		// bbox <- bbox(objeto_geometrico) transformado con la transformacion GLOBAL de este nodo

		// bb = m_gObject->getContainer()
		// bb->transform(m_placementWC)

		// bbox <= T(bbox(objeto_geometrico))

		// void clone(const BBox * source): Copy from source bounding box
		// const BBox *getContainer(); //!< Get bounding box of GObject ESTA EN GEOMETRY/GOBJECT.
		// void transform(const Trfm3D * T): Transform BBox by applying transformation T
	if(m_gObject){
		m_containerWC->clone(m_gObject->getContainer());
		m_containerWC->transform(m_placementWC);
	}else{
		// si es nodo intermedio, bbox  <=  union del bbox de los hijos

		// INICIALIZAR EL BBOX
		//    void init(): Set BBox to be the void (empty) BBox
		m_containerWC->init();
		for(list<Node *>::iterator it = m_children.begin(), end = m_children.end();
		it != end; ++it) {
			Node *theChild = *it;
			m_containerWC->include(theChild->m_containerWC);
			// HACER INCLUDE del bbox del hijo
			// no es una funcion recursiva
			//    void include(BBox *B): Change BBox so that it also includes BBox B

		}
	}


		///////////////////////////////





}

// @@ TODO: Update WC (world coordinates matrix) of a node and
// its bounding box recursively updating all children.
//
// given a node:
//  - update the world transformation (m_placementWC) using the parent's WC transformation
//  - recursive call to update WC of the children
//  - update Bounding Box to world coordinates
//
// Precondition:
//
//  - m_placementWC of m_parent is up-to-date (or m_parent == 0)
//
// Note:
//    See Recipe 1 in for knowing how to iterate through children.

void Node::updateWC() {
	//Lectura del arbol de abajo a arriba, empezando en el nodo raiz
	//Las transformaciones locales y las del mundo son iguales
	if(this->m_parent == 0){
		// NO HAY PADRE... ESTOY EN EL NODO RAIZ
		m_placementWC->clone(m_placement);
	}else{
		// ES UN NODO INTERMEDIO U HOJA
		// T_wc <= padre->T_wc * T_local
		m_placementWC->clone(this->m_parent->m_placementWC);
		m_placementWC->add(m_placement);
	}
	/// Si es un nodo intermedio, se haran llamadas recursivas para sus hijos
	/// Si es un nodo hoja, no entra en el for y sale
	for(list<Node *>::iterator it = m_children.begin(), end = m_children.end();
	it != end; ++it) {
		Node *theChild = *it;
		theChild->updateWC();
	}

	//  - update Bounding Box to world coordinates
	updateBB();

}


	//si es el nodo raiz{
	//	(Las transformaciones locales son las globales)
	//	m_placementWC.clone(m_placement); Hacemos una clonacion de m_placement y nos aseguramos de hacer una copia de m_placement y si lo hicieramos m_placementWC=m_placement solo estariamos haciendo una copia del puntero
	//	}y sino{
	//		la trasnsformacion global es la transformacion global del padre por la transformacion local
	//		m_placementWC <- theParent-> m_placementWC compuesto con m_placement
	//		si tiene hijos{
	//			llamar recursivamente a la funcion updateWC()
	//		
	//}

// @@ TODO:
//
//  Update geometric state of a node.
//
// given a node:
// - Update WC transformation of sub-tree starting at node (updateWC)
// - Propagate Bounding Box to root (propagateBBRoot), starting from the parent, if parent exists.

void Node::updateGS() {
	updateWC();
	// starting from the parent (OJO... DESDE SU PADRE)
	if(m_parent){
		m_parent->propagateBBRoot();
	}
	// HAY QUE VER SI TIENE PADRE... Y SI LO TIENE HACEMOS:
	// 		m_parent->propagateBBRoot();
}


// @@ TODO:
// Draw a (sub)tree.
//
// These functions can be useful:
//
// RenderState *rs = RenderState::instance();
//
// rs->addTrfm(RenderState::modelview, T); // Add T transformation to modelview
// rs->push(RenderState::modelview); // push current matrix into modelview stack
// rs->pop(RenderState::modelview); // pop matrix from modelview stack to current
//
// gobj->draw(); // draw geometry object (gobj)
//
// Note:
//    See Recipe 1 in for knowing how to iterate through children.

// HAcerlo en profundidad para dibujar los objetos
void Node::draw() {

	ShaderProgram *prev_shader = 0;
	RenderState *rs = RenderState::instance();

	if (m_isCulled) return;

	// Set shader (save previous)
	if (m_shader != 0) {
		prev_shader = rs->getShader();
		rs->setShader(m_shader);
	}

	// Print BBoxes
	if(rs->getBBoxDraw() || m_drawBBox)
		BBoxGL::draw( m_containerWC );

	// 1 Guardar en la pila de transformaciones
	//RenderState-> Tiene todo lo que esta en la escena
	//rs->push(RenderState::modelview); 
	//rs->addTrfm(RenderState::modelview, m_placement); 
	//si el objeto es hoja

	if(m_gObject){
	// 	dibujar el objeto geometrico de este nodo
	//	Encolamos y hacemos las transformaciones solo con los nodos hojas, tenemos que eliminar los dos de arriba
		rs->push(RenderState::modelview); 
		rs->addTrfm(RenderState::modelview, m_placementWC); //Para el environment: así mantenemos la matriz del modelo

		// Set model matrix
		rs->loadTrfm(RenderState::model, m_placementWC);
		m_gObject->draw();
		rs->pop(RenderState::modelview);
	}else{
	// dibujar los hijos
		for(list<Node *>::iterator it = m_children.begin(), end = m_children.end(); it != end; ++it) {
			Node *theChild = *it;
			theChild->draw();
		}
	}
	// 5 Desempilar la matriz de trasnsformaciones
	// rs->pop(RenderState::modelview);

	if(prev_shader != 0) {
		// restore shader
		rs->setShader(prev_shader);
	}
}

// Set culled state of a node's children

void Node::setCulled(bool culled) {
	m_isCulled = culled;
	for(list<Node *>::iterator it = m_children.begin(), end = m_children.end();
		it != end; ++it) {
		Node *theChild = *it;
		theChild->setCulled(culled); // Recursive call
	}
}

// @@ TODO: Frustum culling. See if a subtree is culled by the camera, and
//          update m_isCulled accordingly.

void Node::frustumCull(Camera *cam) {
	int frustum = cam->checkFrustum(m_containerWC, 0);
	if(frustum==1) {	// fuera del frustrum
		this->setCulled(true);
	}else if(frustum==0){	//si el BB intersecta al frustrum y llamamos al frustrumCull con los hijos y la camara que nos pasa por parametro (como es la unica camara que he visto y como me imagino que solo trabajaremos con una camara)
		m_isCulled=false;
		for(list<Node *>::iterator it = m_children.begin(), end = m_children.end();
			it != end; ++it) {
			Node *theChild = *it;
			theChild->frustumCull(cam);
			}
	}else{			//esta dentro del frustrum
		this->setCulled(false);
	}
}

// @@ TODO: Check whether a BSphere (in world coordinates) intersects with a
// (sub)tree.
//
// Return a pointer to the Node which collides with the BSphere. 0
// if not collision.
//
// Note:
//    See Recipe 2 in for knowing how to iterate through children inside a const
//    method.

const Node *Node::checkCollision(const BSphere *bsph) const {
	if (!m_checkCollision) return 0;
	
	const Node *resN;

	//llamada a interseccion entre la esfera del avatar y el BB
	//int res = BSphereBBoxIntersect(bsph, m_containerWC);

	//si intersectan ambos comprobamos si es nodo hoja y sino comprobaremos con los hijos
	if(BSphereBBoxIntersect(bsph, m_containerWC) == IINTERSECT){
		//comprobamos si es nodo hoja
		if(m_gObject){
			return this;
		}else{
		//si no es nodo hoja, pasará a mirar los hijos porque es un nodo intermedio
			for(list<Node *>::const_iterator it = m_children.begin(), end = m_children.end();
				it != end; ++it) {
				const Node *theChild = *it;
				resN = theChild->checkCollision(bsph); // or any other thing
				if(resN){
					return theChild;
				}
			}
			return 0;
		}
	}else{
		//si no choca finalmente devolveremos 0
		return 0;
	}
	
}
	// si es nodo hoja
	//    devuelve this
	// sino
	//    si no choca devolvemos 0
	//    diferenciar entre nodos hoja y nodos intermedios
	//    si es intermedio, funcion recursiva con los hijos
	//    si choca devolvemos el hijo(?)
	//
	/* =================== PUT YOUR CODE HERE ====================== */

	/* =================== END YOUR CODE HERE ====================== */

