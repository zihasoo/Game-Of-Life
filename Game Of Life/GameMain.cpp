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
		int r, c, aroundCell=0; //주변에 살아있는 세포의 개수;
		std::pair<int, int> cur;
		std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false)); //방문처리용 보드
		std::queue<std::pair<std::pair<int, int>,bool>> buffer; //보드 업데이트용 버퍼
		std::queue<std::pair<int, int>> Q; //일반 세포들 검사용
		std::queue<std::pair<int, int>> deactivated_Q; //죽은 세포들 검사용
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
							if (ValueBoard[r][c]) { //살아있는 세포가 주변에 있을 경우
								aroundCell++; //살아있는 세포 개수 추가
								if (!visited[r][c]) { //방문을 안했으면 방문 목록에도 추가
									visited[r][c] = true;
									Q.emplace(r, c);
								}
							}
							else if(!visited[r][c]) { //죽은 세포이지만 방문하지 않은 경우
								visited[r][c] = true; //죽은 세포들 방문 목록에 넣어주고 나중에 일괄 검사
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
		while (!deactivated_Q.empty()){ //죽은 세포들 살리기
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

		while (!buffer.empty()){ //검사 종료 후 세포 일괄 업데이트
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
	std::cout << "Mouse Left: 세포 생성" << '\n';
	std::cout << "Mouse Right: 세포 삭제" << '\n';
	std::cout << "Mouse Scroll: 확대/축소" << '\n';
	std::cout << "Space: 일시 정지" << '\n';
	std::cout << "BackSpace: 세포 판 초기화" << '\n';
	std::cout << "R: 확대/축소 초기화" << '\n';
	std::cout << "E: 업데이트 로직 속도 조절" << "\n\n";

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

		if (Mouse::isButtonPressed(Mouse::Left)) { //마우스 왼쪽 클릭으로 세포 추가
			board->clicked(Mouse::getPosition(*window),true);
		}
		if (Mouse::isButtonPressed(Mouse::Right)) { //마우스 오른쪽 클릭으로 세포 삭제
			board->clicked(Mouse::getPosition(*window),false);
		}
		if (Keyboard::isKeyPressed(Keyboard::Space) && CoolTimeMgr::StopCoolDown()) { //메인 업데이트 로직 일시 정지
			pause = !pause;
		}
		if (Keyboard::isKeyPressed(Keyboard::BackSpace) && CoolTimeMgr::ResetCoolDown()) { //화면 클리어
			board->clear();
		}
		if (Keyboard::isKeyPressed(Keyboard::E) && CoolTimeMgr::ResetCoolDown()) { //로직 업데이트 속도 조절
			int x;
			restart:
			std::cout << "로직 업데이트 딜레이 설정(Millisecond): ";
			std::cin >> x;
			if (x < 0) {
				std::cout << "잘못된 숫자\n";
				goto restart;
			}
			CoolTimeMgr::setUpdateCool(x);
			std::cout << "로직 업데이트 딜레이 " << x << " Milliseconds로 설정됨\n\n";
		}
		if (Keyboard::isKeyPressed(Keyboard::R) && CoolTimeMgr::ResetCoolDown()) { //뷰 위치 기본값으로
			viewSize = defaultViewSize, viewPos = viewSize/2;
			view->setCenter(viewPos);
			view->setSize(viewSize);
			window->setView(*view);
		}
		if (CoolTimeMgr::UpdateCoolDown()) { //메인 업데이트 로직
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
