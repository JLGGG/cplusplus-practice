#include <iostream>
#include <memory>

// 스마트포인터인 shared_ptr를 관리해주는 weak_ptr 테스트
// weak_ptr은 shared_ptr의 리소스 관리 시 사용.
// 실제 내부 동작에서 weak_ptr은 리소스를 직접 가지지 않고 단지 레퍼런스만 가짐.
// 주된 사용 목적은 shared_ptr이 삭제될 때 리소스가 해제되었는지 확인하기 위해서 사용함.

class Simple 
{
    public:
    Simple() {
        std::cout<<"Simple constructor called"<<std::endl;
    }
    ~Simple() {
        std::cout<<"Simple destructor called"<<std::endl;
    }    
};

void UseResource(std::weak_ptr<Simple>& weak_simple)
{
    // weak_ptr에 저장된 shared_ptr 포인터에 접근하기 위해서 lock() 메서드를 이용. 
    // lock() 반환 값은 shared_ptr.
    auto resource = weak_simple.lock();
    if (resource) {
        std::cout<<"Resource still alive"<<std::endl;
    } else {
        std::cout<<"Resource has been freed!"<<std::endl;
    }
}

int main()
{
    auto shared_simple = std::make_shared<Simple>();
    std::weak_ptr<Simple> weak_simple(shared_simple);

    UseResource(weak_simple);

    // shared_ptr 리셋 실행 시 레퍼런스 카운팅 메커니즘에 따라 마지막 shared_ptr이 제거되거나 할 때 리소스 해제
    // 현재 코드에서는 shared_ptr이 가지는 리소스가 Simple 객체 하나뿐이므로 리셋 시 리소스가 해제됨.
    shared_simple.reset();

    // weak_ptr을 호출해보면 shared_ptr이 가지고 있던 리소스가 해제된 것을 확인할 수 있음.
    UseResource(weak_simple);

    return 0;

}