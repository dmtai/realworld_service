import pytest

from testsuite.databases import pgsql


# Start the tests via `make test-debug` or `make test-release`


@pytest.mark.parametrize(
    'user',
    [
        (
            {"user": {
                "username": "Jacob",
                "email": "jake@jake.jake",
                "password": "jakejake"
            }
            }
        )
    ],
)
async def test_user_registration(service_client, user):
    response = await service_client.post(
        "/api/users", json=user,
    )
    assert response.status == 200
    assert response.json()["user"]["email"] == user["user"]["email"]
    assert response.json()["user"]["username"] == user["user"]["username"]
    assert response.json()["user"]["bio"] is None
    assert response.json()["user"]["image"] is None
    assert response.json()["user"]["token"] != ""
