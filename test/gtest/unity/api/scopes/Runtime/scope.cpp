#include <scopes/Runtime.h>
#include <scopes/ScopeBase.h>

using namespace unity::api::scopes;

class TestScope : public ScopeBase
{
public:
    virtual int start(std::string const&, RegistryProxy const &) override
    {
        return VERSION;
    }

    virtual void stop() override
    {
    }

    virtual void run() override
    {
    }

    virtual QueryBase::UPtr create_query(std::string const &, VariantMap const &) override
    {
        return nullptr;
    }
};

int main(int, char **argv) {
    auto rt = Runtime::create("TestScope", argv[1]);
    TestScope scope;
    rt->run_scope(&scope);
    return 0;
}
