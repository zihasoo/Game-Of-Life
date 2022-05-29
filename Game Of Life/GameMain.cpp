#include <SFML/Graphics.hpp>
#include <vector>
#include <queue>
#include <iostream>
using namespace sf;

Vector2f operator/(const Vector2f& a, const int& b) {
	return { a.x / b, a.y / b };
}

void operator-=(Vector2f& a, const int& b) {
	a.x -= b, a.y -= b;
}

const int windowWidth = 800, windowHeight = 800;
const Vector2f defaultViewSize(400, 400);
Vector2f viewSize(defaultViewSize), viewPos(viewSize/2);
int curTime = 0;
bool pause = false;

class GameBoard : public Drawable, public Transformable{
	int tileSize = 10;
	int width = windowWidth/tileSize, height = windowHeight/tileSize;
	std::pair<int,int> dir[8] = { {1,0}, {1,1}, {0,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1}, {-1,1} };
	VertexArray ImageBoard;
	std::vector<std::vector<bool>> ValueBoard;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		states.transform *= getTransform();
		target.draw(ImageBoard, states);
	}
public:
	GameBoard() {
		Vertex* quad;
		ValueBoard.assign(height, std::vector<bool>(width, false));
		ImageBoard.setPrimitiveType(Quads);
		ImageBoard.resize(height * width * 4);
		for (int i = 0; i < height; i++){
			for (int j = 0; j < width; j++){
				quad = &ImageBoard[(i * width + j) * 4];
				quad[0].position = Vector2f(j * tileSize, i * tileSize);
				quad[1].position = Vector2f((j+1) * tileSize-1, i * tileSize);
				quad[2].position = Vector2f((j + 1) * tileSize-1, (i + 1) * tileSize-1);
				quad[3].position = Vector2f(j * tileSize, (i+1) * tileSize-1);
				quad[0].color = Color::Black;
				quad[1].color = Color::Black;
				quad[2].color = Color::Black;
				quad[3].color = Color::Black;
			}
		}
	}
	
	void clicked(Vector2i pos, bool value) {
		int x = pos.x / (tileSize * (windowWidth / viewSize.x));
		int y = pos.y / (tileSize * (windowHeight / viewSize.y));
		if (x < 0 || x >= width || y < 0 || y >= height) return;
		ValueBoard[y][x] = value;
		imageBoardUpdate(y, x);
	}

	void clear() {
		Vertex* quad;
		ValueBoard.assign(height, std::vector<bool>(width, false));
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				quad = &ImageBoard[(i * width + j) * 4];
				quad[0].color = Color::Black;
				quad[1].color = Color::Black;
				quad[2].color = Color::Black;
				quad[3].color = Color::Black;
			}
		}
	}

	void imageBoardUpdate(int r, int c) {
		Color temp = (ValueBoard[r][c] ? Color::White : Color::Black);
		ImageBoard[(r * width + c) * 4].color = temp;
		ImageBoard[(r * width + c) * 4 + 1].color = temp;
		ImageBoard[(r * width + c) * 4 + 2].color = temp;
		ImageBoard[(r * width + c) * 4 + 3].color = temp;
	}

	void mainUpdate() {
		int r, c, aroundCell=0; //�ֺ��� ����ִ� ������ ����;
		std::pair<int, int> cur;
		std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false)); //�湮ó���� ����
		std::queue<std::pair<std::pair<int, int>,bool>> buffer; //���� ������Ʈ�� ����
		std::queue<std::pair<int, int>> Q; //�Ϲ� ������ �˻��
		std::queue<std::pair<int, int>> deactivated_Q; //���� ������ �˻��
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				if (ValueBoard[i][j] && !visited[i][j]) {
					visited[i][j] = true;
					Q.emplace(i,j);
					while (!Q.empty()){
						aroundCell = 0;
						cur = Q.front(); Q.pop();
						for (std::pair<int, int> d : dir) {
							r = cur.first + d.first; c = cur.second + d.second;
							if (r < 0 || r >= height || c < 0 || c >= width) continue;
							if (ValueBoard[r][c]) { //����ִ� ������ �ֺ��� ���� ���
								aroundCell++; //����ִ� ���� ���� �߰�
								if (!visited[r][c]) { //�湮�� �������� �湮 ��Ͽ��� �߰�
									visited[r][c] = true;
									Q.emplace(r, c);
								}
							}
							else if(!visited[r][c]) { //���� ���������� �湮���� ���� ���
								visited[r][c] = true; //���� ������ �湮 ��Ͽ� �־��ְ� ���߿� �ϰ� �˻�
								deactivated_Q.emplace(r, c);
							}
						}
						if (aroundCell < 2 || aroundCell > 3) {
							buffer.emplace(cur,false);
						}
					}
				}
			}
		}
		while (!deactivated_Q.empty()){ //���� ������ �츮��
			aroundCell = 0;
			cur = deactivated_Q.front(); deactivated_Q.pop();
			for (std::pair<int, int> d : dir) {
				r = cur.first + d.first; c = cur.second + d.second;
				if (r < 0 || r >= height || c < 0 || c >= width) continue;
				if (ValueBoard[r][c]) {
					aroundCell++;
				}
			}
			if (aroundCell == 3) {
				buffer.emplace(cur, true);
			}
		}

		while (!buffer.empty()){ //�˻� ���� �� ���� �ϰ� ������Ʈ
			std::pair<std::pair<int, int>, bool> BC = buffer.front();
			buffer.pop();
			ValueBoard[BC.first.first][BC.first.second] = BC.second;
			imageBoardUpdate(BC.first.first, BC.first.second);
		}
	}
};

class CoolTimeMgr {
	static int UpdateCool;
public:
	static void setUpdateCool(int x) {
		UpdateCool = x;
	}
	static bool StopCoolDown() {
		static int lastButtonPressedTime = 0, PressCool = 200;
		if ((curTime - lastButtonPressedTime) > PressCool) {
			lastButtonPressedTime = curTime;
			return true;
		}
		return false;
	}
	static bool ResetCoolDown() {
		static int lastButtonPressedTime = 0, PressCool = 200;
		if ((curTime - lastButtonPressedTime) > PressCool) {
			lastButtonPressedTime = curTime;
			return true;
		}
		return false;
	}
	static bool UpdateCoolDown() {
		static int lastUpdatedTime = 0;
		if (curTime - lastUpdatedTime > UpdateCool && !pause) {
			lastUpdatedTime = curTime;
			return true;
		}
		return false;
	}
};

int CoolTimeMgr::UpdateCool = 200;

int main() {
	std::cout << "Mouse Left: ���� ����" << '\n';
	std::cout << "Mouse Right: ���� ����" << '\n';
	std::cout << "Mouse Scroll: Ȯ��/���" << '\n';
	std::cout << "Space: �Ͻ� ����" << '\n';
	std::cout << "BackSpace: ���� �� �ʱ�ȭ" << '\n';
	std::cout << "R: Ȯ��/��� �ʱ�ȭ" << '\n';
	std::cout << "E: ������Ʈ ���� �ӵ� ����" << "\n\n";

	Vector2i beforeMousePos;
	Clock *clock = new Clock();
	View* view = new View(viewPos,viewSize);
	GameBoard* board = new GameBoard();
	RenderWindow* window = new RenderWindow(VideoMode(windowWidth, windowHeight), "Game Of Life");
	window->setFramerateLimit(144);
	window->setView(*view);

	while (window->isOpen()) {
		Event e;
		while (window->pollEvent(e)) {
			if (e.type == Event::Closed) window->close();

			if (e.type == Event::MouseWheelScrolled  
				&& viewSize.x - e.mouseWheelScroll.delta * 20 <= windowWidth
				&& viewSize.x - e.mouseWheelScroll.delta * 20 >= 10) {

				viewSize -= e.mouseWheelScroll.delta * 20;
				viewPos = viewSize / 2;
				view->setCenter(viewPos);
				view->setSize(viewSize);
				window->setView(*view);
			}
		}
		window->clear(Color(150,150,150));
		curTime = clock->getElapsedTime().asMilliseconds();

		if (Mouse::isButtonPressed(Mouse::Left)) { //���콺 ���� Ŭ������ ���� �߰�
			board->clicked(Mouse::getPosition(*window),true);
		}
		if (Mouse::isButtonPressed(Mouse::Right)) { //���콺 ������ Ŭ������ ���� ����
			board->clicked(Mouse::getPosition(*window),false);
		}
		if (Keyboard::isKeyPressed(Keyboard::Space) && CoolTimeMgr::StopCoolDown()) { //���� ������Ʈ ���� �Ͻ� ����
			pause = !pause;
		}
		if (Keyboard::isKeyPressed(Keyboard::BackSpace) && CoolTimeMgr::ResetCoolDown()) { //ȭ�� Ŭ����
			board->clear();
		}
		if (Keyboard::isKeyPressed(Keyboard::E) && CoolTimeMgr::ResetCoolDown()) { //���� ������Ʈ �ӵ� ����
			int x;
			restart:
			std::cout << "���� ������Ʈ ������ ����(Millisecond): ";
			std::cin >> x;
			if (x < 0) {
				std::cout << "�߸��� ����\n";
				goto restart;
			}
			CoolTimeMgr::setUpdateCool(x);
			std::cout << "���� ������Ʈ ������ " << x << " Milliseconds�� ������\n\n";
		}
		if (Keyboard::isKeyPressed(Keyboard::R) && CoolTimeMgr::ResetCoolDown()) { //�� ��ġ �⺻������
			viewSize = defaultViewSize, viewPos = viewSize/2;
			view->setCenter(viewPos);
			view->setSize(viewSize);
			window->setView(*view);
		}
		if (CoolTimeMgr::UpdateCoolDown()) { //���� ������Ʈ ����
			board->mainUpdate();
		}

		beforeMousePos = Mouse::getPosition(*window);
		window->draw(*board);
		window->display();
	}

	delete clock; delete view;
	delete window; delete board;
	return 0;
}
